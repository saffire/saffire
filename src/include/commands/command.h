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
#ifndef __COMMAND_H__
#define __COMMAND_H__

    #include "general/parse_options.h"

    /*
     * Saffire uses a non-posix argument structure, but something similar
     * like git, which uses command arguments and optional flags.
     *
     * ./saffire <command> [action [options]] [--flags]
     *
     * There are a few exceptions:
     *
     *  - running ./saffire without options will trigger the cli mode
     *  - running ./saffire <file> will trigger the exec, if the file exists
     *  - running ./saffire --<arg> will strip away the dashes and use it as a command. This allows ./saffire -h  etc..
     *
     */

    struct command {
        char *name;                         // Command name
        struct command_info *info;          // Details for the command (including the actions)
    };

    struct command_action {
        char *name;                         // Action
        char *arglist;                      // Argument list
        int (*func)(void);                  // Function to call when format matches
        struct saffire_option *options;     // Pointer of options handlers
    };

    struct command_info {
        const char *description;            // Description for this command.
        struct command_action *actions;     // Defined actions
        const char *help;                   // Additional help text (or NULL)
    };


#endif
