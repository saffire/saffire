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
#include <unistd.h>
#include <sys/stat.h>
#include "objects/object.h"
#include "modules/module_api.h"
#include "compiler/ast.h"
#include "general/smm.h"
#include "commands/command.h"
#include "general/parse_options.h"
#include "general/path_handling.h"
#include "vm/vm.h"
#include "compiler/ast_walker.h"
#include "compiler/assembler.h"
#include "general/output.h"
#include "debug.h"

static int flag_no_verify = 0;

static int do_exec(void) {
    char *source_file = saffire_getopt_string(0);
    struct stat source_stat;
    t_bytecode *bc;

    // Check if sourcefile exists
    if (stat(source_file, &source_stat) != 0) {
        error("Source file '%s' does not exist.\n", source_file);
        return 1;
    }

    // Check if bytecode exists, or has a correct timestamp
    char *bytecode_file = replace_extension(source_file, ".sf", ".sfc");
    int bytecode_exists = (access(bytecode_file, F_OK) == 0);

    if (! bytecode_exists || bytecode_get_timestamp(bytecode_file) != source_stat.st_mtime) {
        // (Re)generate bytecode file
        t_ast_element *ast = ast_generate_from_file(source_file);
        if (! ast) {
            error("Cannot create AST");
            smm_free(bytecode_file);
            return 1;
        }
        t_dll *asm_code = ast_walker(ast);
        if (! asm_code) {
            error("Cannot create assembler");
            smm_free(bytecode_file);
            return 1;
        }
        bc = assembler(asm_code);
        bytecode_save(bytecode_file, source_file, bc);
    } else {
        bc = bytecode_load(bytecode_file, flag_no_verify);
    }

    // Something went wrong with the bytecode loading or generating
    if (!bc) {
        error("Error while loading bytecode\n");
        smm_free(bytecode_file);
        return 1;
    }

    smm_free(bytecode_file);



    vm_init();
    int exitcode = vm_execute(bc);
    vm_fini();

    bytecode_free(bc);
    DEBUG_PRINT("VM ended with exitcode: %d\n", exitcode);

    return exitcode;
}


/****
 * Argument Parsing and action definitions
 ***/

/* Usage string */
static const char help[]   = "Executes a Saffire script.\n"
                             "\n"
                             "Global settings:\n"
                             "   --no-verify      Don't verify signature from bytecode file (if any)\n"
                             "\n"
                             "This command executes a saffire script or bytecode file.\n";


static void opt_no_verify(void *data) {
    flag_no_verify = 1;
}

static struct saffire_option exec_options[] = {
    { "no-verify", "", no_argument, opt_no_verify},
};

/* Config actions */
static struct command_action command_actions[] = {
    { "", "s", do_exec, exec_options },
    { 0, 0, 0, 0 }
};

/* Config info structure */
struct command_info info_exec = {
    "Execute saffire script",
    command_actions,
    help
};
