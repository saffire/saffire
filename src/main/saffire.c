/*
 Copyright (c) 2012-2013, The Saffire Group
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
#include <sys/stat.h>
#include "general/output.h"
#include "general/parse_options.h"
#include "commands/command.h"


#include "general/hashtable.h"

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

// Original argc and argv. Since we are moving around the argument pointers.
int original_argc;
char **original_argv;



/**
 * Prints usage information
 */
void print_usage(void) {
    output("\n"
           "Usage: saffire <command> [options] [script [args]]\n"
           "Available options are:\n"
           "  -v, --version         Show version information \n"
           "  -h, --help            This usage information \n"
           "\n"
           "Available commands:\n");

    struct command *p = commands;
    while (p->name) {
        output("%-15s %s\n", p->name, p->info->description);
        p++;
    }
    output("\n");

    output("Use saffire help <command> to find out more information about the command usage.\n\n");
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
            if (action->options) {
                saffire_parse_options(argc, argv, &action->options);
            }

            // Parse the rest of the arguments, confirming the action's signature
            char *error;
            if (! saffire_parse_signature(argc, argv, action->arglist, &error)) {
                fatal_error(1, "%s. Use 'saffire help %s' for more information\n", error, action->name);
            }

            // Execute action
            return action->func();
        }

        action++;
    }

    // Nothing was found. Display help if available.
    if (cmd->info->help) {
        output("%s", cmd->info->help);
        output("\n");
    } else {
        output("No additional help is available. Use 'saffire help' for more information.\n");
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


/**
 * Main Saffire entry point
 */
int main(int argc, char *argv[]) {
    char *command = "";

    // Save originals. Commands like 'help' will need them..
    original_argc = argc;
    original_argv = argv;

    // Find command, and set the argc & argv to point to the item AFTER the command
    if (argc == 1) {
        // Turn "./saffire" into "./saffire help"
        command = "help";
        argc = 0;
    } else if (argc >= 2) {
        // If the action is 'dashed', remove the dashes. This will change
        // "saffire --help", into "saffire help"
        while (argv[1][0] == '-') argv[1]++;
        command = argv[1];
        argv += 2;
        argc -= 2;
    }


    // Iterate commands and see if we have a match and run it.
    struct command *p = commands;
    while (p->name) {
        if (! strcmp(p->name, command)) {
            exit(_exec_command(p, argc, argv));
        }
        p++;
    }


    /* No correct command found. We do a simple test to see our "command"
     * actually matches a file. If so, we do a "exec" instead
     */
    struct stat buf;
    int status = stat(command, &buf);
    if (status == 0) {
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
