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
#include "general/output.h"
#include "commands/command.h"

extern int original_argc;
extern char **original_argv;
extern void print_usage(void);
extern struct command commands[];

/**
 * Display help usage
 */
static int do_help(void) {
    if (original_argc <= 2) {
        print_usage();
        return 0;
    }

    struct command *cmd = commands;
    while (cmd->name) {

        if (! strcasecmp(cmd->name, original_argv[2])) {
            if (cmd->info->help) {
                output("%s\n", cmd->info->help);
            } else {
                // No help available for this command
                output("There is no help available for command '%s'.\n", original_argv[2]);
            }
            return 0;
        }

        cmd++;
    }

    // No valid command found
    output("Cannot find help on this subject.\n");
    return 1;
}


/****
 * Argument Parsing and action definitions
 ***/


/* Usage string */
static const char help[]   = "Display help information\n"
                             "\n";


/* Config actions */
static struct command_action command_actions[] = {
    { "", "|s", do_help, NULL },
    { 0, 0, 0, 0 }
};

/* Config info structure */
struct command_info info_help = {
    "help",
    command_actions,
    help
};
