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

#include <glob.h>
#include <stdio.h>
#include <string.h>
#include "general/output.h"
#include "commands/command.h"
#include "compiler/ast_nodes.h"
#include "general/parse_options.h"
#include "general/path_handling.h"
#include "general/smm.h"

void process_file(const char *filename);
void process_directory(const char *directory);
int check_file(const char *filename);

/**
 * Start the lint process
 */
static int do_lint(void) {
    char *target = saffire_getopt_string(0);

    if(is_file(target)) {
        process_file(target);
    } else if (is_directory(target)) {
        output("Checking directory: %s.\n", target);
        process_directory(target);
    } else {
        output("Received neither a file nor a directory.\n");
    }

    return 0;
}


/**
 * Perform a syntax check on a single file
 */
void process_file(const char *filename) {
    if ( ! is_saffire_file(filename)) {
        output("%s is not a saffire file.\n", filename);
    } else {
        check_file(filename);
    }
}

/**
 * Syntax check the contents of a directory
 */
void process_directory(const char *directory) {
    glob_t buffer;
    char *extension = "/*";
    char *pattern = smm_malloc(sizeof(directory) + sizeof(extension) + 1);
    strcpy(pattern, directory);
    strcat(pattern, extension);
    glob(pattern, 0, NULL, &buffer);
    smm_free(pattern);

    for (int i = 0; i < buffer.gl_pathc; i++) {
        if (is_file(buffer.gl_pathv[i])) {
            process_file(buffer.gl_pathv[i]);
        } else if (is_directory(buffer.gl_pathv[i])) {
            process_directory(buffer.gl_pathv[i]);
        }
    }
}

/**
 * perform the actual syntax check
 */
int check_file(const char *filename) {
    output("checking %s.\n", filename);
    t_ast_element *ast = ast_generate_from_file((char*)filename);

    if (ast == NULL) {
        output(" - %s contains syntax errors.\n", filename);
        return 0;
    }
    return 1;
}

/****
 * Argument Parsing and action definitions
 ***/


/* Usage string */
static const char help[]   = "Lint check a Saffire source file or directory\n"
                             "\n";

/* Config actions */
static struct command_action command_actions[] = {
    { "", "s", do_lint, NULL },
    { 0, 0, 0, 0 }
};

struct command_info info_lint = {
    "Perform lint check",
    command_actions,
    help
};
