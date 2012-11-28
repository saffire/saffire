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
#include <getopt.h>
#include <stdlib.h>
#include <locale.h>
#include "objects/object.h"
#include "modules/module_api.h"
#include "compiler/ast.h"
#include "dot/dot.h"
#include "commands/command.h"
#include "general/parse_options.h"
#include "vm/vm.h"

char *dot_file = NULL;

static int do_exec(void) {
    char *source_file = saffire_getopt_string(0);

    setlocale(LC_ALL,"");
    vm_init();

    t_ast_element *ast = ast_generate_from_file(source_file);

    if (dot_file) {
        dot_generate(ast, dot_file);
    }

    // @TODO: here be interpreting
    int ret = 0;
    ast_walker(ast);

    // Release memory of ast root
    if (ast != NULL) {
        ast_free_node(ast);
    }

    vm_fini();

    return ret;
}


/****
 * Argument Parsing and action definitions
 ***/

static void opt_dot(void *data) {
    dot_file = (char *)data;
}


/* Usage string */
static const char help[]   = "Executes a Saffire script.\n"
                             "\n"
                             "Global settings:\n"
                             "    --dot, -d <FILE>        Generate a DOT file\n"
                             "\n"
                             "This command allows you to enter Saffire commands, which are immediately executed.\n";


static struct saffire_option global_options[] = {
    { "dot", "d", required_argument, opt_dot },
    { 0, 0, 0, 0 }
};


/* Config actions */
static struct command_action command_actions[] = {
    { "", "s", do_exec, global_options },
    { 0, 0, 0, 0 }
};

/* Config info structure */
struct command_info info_exec = {
    "Execute saffire script",
    command_actions,
    help
};
