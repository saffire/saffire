/*
 Copyright (c) 2012-2015, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the Saffire Group the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <saffire/general/output.h>
#include <saffire/general/parse_options.h>
#include <saffire/commands/command.h>
#include <saffire/general/smm.h>
#include <saffire/general/hashtable.h>
#include <saffire/general/config.h>

char DEFAULT_EXTERNAL_COMMAND_PATH[] = "/usr/lib/saffire/commands";

/*
 * Info structures for the Saffire commands
 */
extern struct command_info info_version;
extern struct command_info info_help;
extern struct command_info info_lint;
extern struct command_info info_repl;
extern struct command_info info_fastcgi;
extern struct command_info info_config;
extern struct command_info info_exec;
extern struct command_info info_bytecode;

// Each saffire "command" must have a entry here, otherwise it's not known.
struct command commands[] = {
                                        { "help",     &info_help },
                                        { "version",  &info_version },
                                        { "lint",     &info_lint },
                                        { "repl",     &info_repl },
                                        { "fastcgi",  &info_fastcgi },
                                        { "config",   &info_config },
                                        { "exec",     &info_exec },
                                        { "bytecode", &info_bytecode },
                                        { NULL, NULL }
                                };



static void opt_config(void *data) {
    config_which_ini = USE_INI_CUSTOM;
    config_custom_ini_path = string_strdup0((char *)data);
}

static void opt_global(void *data) {
    config_which_ini = USE_INI_GLOBAL;
}

static void opt_local(void *data) {
    config_which_ini = USE_INI_LOCAL;
}

static struct saffire_option global_options[] = {
    { "config", "", required_argument, opt_config },
    { "global", "", no_argument, opt_global },
    { "local",  "", no_argument, opt_local },
    { 0, 0, 0, 0 }
};



// Original argc and argv. Since we are moving around the argument pointers.
int original_argc;
char **original_argv;


/**
 * Prints usage information
 */
void print_usage(void) {
    output_char("Usage: saffire <command> [options] [script -- [args]]\n"
           "\n"
           "Global arguments:\n"
           "    --config <filename>   Configuration file to read\n"
           "    --global              Use global configuration\n"
           "\n"
           "Available commands:\n");

    // Display all available commands
    struct command *p = commands;
    while (p->name) {
        output_char("  %-15s %s\n", p->name, p->info->description);
        p++;
    }
    output_char("\n");

    output_char("Use saffire help <command> to find out more information about the command usage.\n\n");
}


/**
 * Execute a command.
 */
static int _exec_command (struct command *cmd, int argc, char **argv) {
    // argv[0] points to action (or start of parameter list, or NULL when no actions/params are given)
    char *dst_action = argv[0] ? argv[0] : "";

    // Iterate structure and find correct action depending on the argument signature
    struct command_action *action = cmd->info->actions;
    while (action->name) {

        // Match action or an empty action
        if (! strcmp(action->name, "") || ! strcmp(action->name, dst_action)) {

            // Remove the action from the argument list, if we found an action
            if (strcmp(action->name, "")) {
                argv += 1;
                argc -= 1;
            }

            // Parse options clears the options as soon as they are processed
            if (action->options && argc) {
                saffire_parse_options(&argc, argv, &action->options, 1);
            }

            // Parse the rest of the arguments, confirming the action's signature
            char *error = NULL;
            if (! saffire_parse_signature(argc, argv, action->arglist, &error)) {
                output_char("%s\n", error);
                output_char("%s", cmd->info->help);
                output_char("\n");
                if (error) smm_free(error);
                return 1;
            }

            // Execute action
            return action->func();
        }

        action++;
    }

    // Not even an external command was found. Display help if available.
    if (cmd->info->help) {
        output_char("%s", cmd->info->help);
        output_char("\n");
    } else {
        output_char("No additional help is available. Use 'saffire help' for more information.\n");
    }

    return 1;
}


/**
 *
 */
static const char *get_filename_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

static int has_filename_extension(const char *filename, const char *extension) {
    return (strcmp(get_filename_extension(filename), extension) == 0);
}


// Find command, and set the argc & argv to point to the item AFTER the command
static char *parse_command_line(int *argc, char **argv[]) {
    // Turn "./saffire" into "./saffire help"
    if (*argc == 1) {
        (*argv) += 1;
        *argc -= 1;

        return "help";
    }

    // Special cases for -h, --help, -?
    if (strcmp((*argv)[1], "-h") == 0 ||
        strcmp((*argv)[1], "--help") == 0 ||
        strcmp((*argv)[1], "-?") == 0) {
        // Since "" is not a command, it will fall through commands and display usage

        // assume it's the only argument given
        (*argv) += 2;
        *argc -= 2;

        return "help";
    }

    // Special case for -v
    if (strcmp((*argv)[1], "-v") == 0) {
        // assume it's the only argument given
        (*argv) += 2;
        *argc -= 2;

        return "version";
    }

    // We parse and get rid of any global values
    struct saffire_option *opts = global_options;
    saffire_parse_options(argc, *argv, &opts, 0);

    // We assume the first argument is a command
    char *command = (*argv)[1];

    // increase argv|argc, as we need to point to the first item AFTER the command
    (*argv) += 2;
    *argc -= 2;

    // If "command" ends on ".sf" or ".sfc", we assume we need to execute it.
    if (has_filename_extension(command, "sf") || has_filename_extension(command, "sfc")) {
        // Reset argc|argv to point to the filename
        (*argv) -= 1;
        *argc += 1;
        return "exec";
    }

    return command;
}


/**
 * Main Saffire entry point
 */
int main(int argc, char *argv[]) {
    // Save originals. Commands like 'help' will need them..
    original_argc = argc;
    original_argv = argv;

    char *command = parse_command_line(&argc, &argv);

    // Read configuration first
    if (! config_read() && config_which_ini != USE_INI_SEARCH) {
        output_char("Warning: cannot read configuration '%s/saffire.ini' \n", config_get_path());
    }

    // Iterate commands and see if we have a match and run it.
    struct command *p = commands;
    while (p->name) {
        if (! strcmp(p->name, command)) {
            exit(_exec_command(p, argc, argv));
        }
        p++;
    }


    // Nothing was found, try external commands
    char *path = config_get_string("global.saffire.command.path", DEFAULT_EXTERNAL_COMMAND_PATH);

    char *fullpath;
    smm_asprintf_char(&fullpath, "%s/saffire-%s", path, command);

    if( access(fullpath, F_OK) != -1 ) {
        char *environ[] = { NULL };
        execve(fullpath, argv, environ);

        output_char("execve() errored on %s\n", fullpath);
        smm_free(fullpath);
        return 1;
    }


    /* No correct command found. We do a simple test to see our "command"
     * actually matches a file. If so, we do a "exec" instead
     */
    struct stat buf;
    int status = stat(command, &buf);
    if (status == 0) {
        // If "command" ends on ".sf" or ".sfc" and it's a valid file, we assume
        // we need to execute it.
        command = "exec";

        // Rescan commands for "Exec", and run
        struct command *p = commands;
        while (p->name) {
            if (! strcmp(p->name, command)) {
                // "push" back the filename and execute
                exit(_exec_command(p, argc+1, argv-1));
            }
            p++;
        }
    }


    // Did not found anything to execute. Give some usage help.
    print_usage();
    return 1;
}
