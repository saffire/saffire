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
#include <sys/stat.h>
#include "commands/command.h"
#include "general/parse_options.h"
#include "compiler/bytecode.h"


int flag_remove = 0;


/**
 * Add or remove signature from a module
 */
static int _sign_module(const char *path) {
    // @TODO: implement this
    return 0;
}


/**
 * Add or remove signature from a bytecode file
 */
static int _sign_bytecode(const char *path) {

    if (! bytecode_is_valid_file(path)) {
        printf("This is not a valid saffire bytecode file.");
        return 1;
    }

    if (flag_remove == 1) {
        if (! bytecode_is_signed(path)) {
            printf("This bytecode file is not signed.");
            return 1;
        }
        // Remove signature
        return bytecode_remove_signature(path);
    }

    if (flag_remove == 0) {
        if (bytecode_is_signed(path)) {
            printf("This bytecode file is already signed.");
            return 1;
        }
        // add signature
        return bytecode_add_signature(path);
    }

    return 0;
}


/**
 *
 */
static int do_sign(void) {
    char *source_path = saffire_getopt_string(0);

    struct stat st;
    if (stat(source_path, &st) != 0) {
        printf("File not found");
        return 1;
    }

    // Compile directory if path matches a directory
    if (S_ISDIR(st.st_mode)) {
        return _sign_module(source_path);
    } else {
        return _sign_bytecode(source_path);
    }

    return 0;
}


/****
 * Argument Parsing and action definitions
 ***/


/* Usage string */
static const char help[]  = "Signs a saffire bytecode file or module.\n"
                            "\n"
                            "Usage: saffire sign <dir>|<file> [options]\n"
                            "\n"
                            "This command signs a saffire bytecode file or module.\n";

static void opt_remove(void *data) {
    flag_remove = 1;
}

static struct saffire_option global_options[] = {
    { "remove", "", no_argument, opt_remove },
    { 0, 0, 0, 0 }
};


/* Config actions */
static struct command_action command_actions[] = {
    { "", "s", do_sign, global_options },
    { 0, 0, 0, 0 }
};

/* Config info structure */
struct command_info info_sign = {
    "Sign saffire bytecode or modules",
    command_actions,
    help
};
