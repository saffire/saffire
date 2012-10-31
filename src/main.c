/*
 Copyright (c) 2012, The Saffire Group
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the <organization> nor the
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
#include <locale.h>
#include "dot/dot.h"
#include "compiler/ast.h"
#include "general/smm.h"
#include "general/parse_options.h"
#include "object/object.h"
#include "interpreter/saffire_interpreter.h"
#include "interactive/interactive.h"
#include "modules/module_api.h"
#include "interpreter/context.h"
#include "version.h"
#include "command.h"

#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

// Some forward defines
int cmd_help(void);
int cmd_version(void);

/*
 * Info structures for the Saffire commands
 */
struct _command_info info_help = { "Displays help information", cmd_help, NULL, NULL, "Displays Saffire usage information." };
struct _command_info info_version = { "Displays version", cmd_version, NULL, NULL, "Displays Saffire version number." };
extern struct _command_info info_lint;
extern struct _command_info info_cli;
extern struct _command_info info_fastcgi;
extern struct _command_info info_config;
extern struct _command_info info_exec;

// Each saffire "command" must have a entry here, otherwise it's not known.
struct _command {
    char *name;                      // Command name
    struct _command_info *info;      // Info structure
};
static struct _command commands[] = {
                                        { "help",    &info_help },
                                        { "version", &info_version },
                                        { "lint",    &info_lint },
                                        { "cli",     &info_cli },
                                        { "fastcgi", &info_fastcgi },
                                        { "config",  &info_config },
                                        { "exec",    &info_exec }
                                    };


/**
 * Prints current version number and copyright information
 */
void print_version(void) {
    printf("%s  - %s\n%s\n", saffire_version, saffire_copyright, saffire_compiled);
}


/**
 * Prints usage information
 */
void print_usage(void) {
    printf("\n"
           "Usage: saffire <command> [options] [script [args]]\n"
           "Available options are:\n"
           "  -v, --version         Show version information \n"
           "  -h, --help            This usage information \n"
           "      --dot <output>    Generate an AST in .dot format\n"
           "\n"
           "Available commands:\n");

    for (int i=0; i<ARRAY_SIZE(commands); i++) {
        struct _command *p = commands+i;
        printf("%-15s %s\n", p->name, p->info->description);
    }
    printf("\n");

    printf("Use saffire help <command> to find out more information about the command usage.\n\n");
}


/**
 * Display help usage
 */
int cmd_help(void) {
    // TODO: fix this
//    if (argc <= 2) {
//        print_usage();
//        return 0;
//    }
//
//    for (int i=0; i < ARRAY_SIZE(commands); i++) {
//        struct _command *cmd = commands+i;
//        if (strcmp(cmd->name, argv[2]) != 0) continue;
//        if (cmd->info->help) {
//            printf("%s\n", cmd->info->help);
//        } else {
//            // No help available for this command
//            printf("There is no help available for command '%s'.\n", argv[2]);
//        }
//        return 0;
//    }

    // No valid command found
    printf("Cannot find help on this subject.\n");
    return 1;
}


/**
 * Display version
 */
int cmd_version(void) {
    print_version();
    return 0;
}


/**
 * Execute a command.
 */
static int _exec_command (struct _command *cmd, int argc, char **argv) {
    // Single command, just execute without any additional checks
    if (cmd->info->func) {
        // Set generic options (note: this will change around the arguments
        saffire_parse_options(argc, argv, &cmd->info->options);

        // Execute function
        return cmd->info->func();
    }

    // Iterate structure, find correct action depending on the argument signature
    int i=0;
    struct _argformat *format = cmd->info->formats + i;
    while (format->action) {
        if (! strcmp(format->action, argv[0])) {

            // Parse options clears the options as soon as they are processed
            saffire_parse_options(argc-1, argv+1, &format->options);

            // Parse the rest of the arguments, confirming the action's signature
            saffire_parse_signature(argc-1, argv+1, format->arglist);

            // Execute action
            return format->func();
        }
        i++;
        format = cmd->info->formats + i;
    }

    // Nothing was found. Display help if available.
    if (cmd->info->help) {
        printf(cmd->info->help);
        printf("\n");
    } else {
        printf("No additional help is available. Use 'saffire help' for more information.\n");
    }
    return 1;
}



/**
 * Main Saffire entry point
 */
int main(int argc, char *argv[]) {
    char *cmd;
    int ret = -1;

    // Turn "./saffire" into "./saffire help"
    if (argc == 1) {
        cmd = "help";
        argc = 1;
        argv[0] = cmd;
    } else {
        cmd = argv[1];
        argv += 2;
        argc -= 2;
    }

/*
    if (argc >= 2) {
        // "./saffire --command ..." will become "./saffire command ..."
        while (argv[1][0] == '-') {
            argv[1] = (char *)(argv[1])+1;
        }
        cmd = argv[1];
    }
*/

    // Iterate all commands, see if we have a match and run it.
    for (int i=0; i < ARRAY_SIZE(commands); i++) {
        struct _command *p = commands+i;
        if (strcmp(p->name, cmd)) continue;

        exit(_exec_command(p, argc, argv));
    }

    // Did not found a command. Give some usage help.
    print_usage();
    return 1;
}
