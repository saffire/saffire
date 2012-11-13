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
#include "interpreter/context.h"
#include "modules/module_api.h"
#include "vm/vm.h"
#include "commands/command.h"
#include "general/parse_options.h"


int flag_sign = 0;      // 0 = default config setting, 1 = force sign, 2 = force unsigned
int flag_compress = 0;  // 0 = default config setting, 1 = force compress, 2 = force uncompressed


static int do_compile(void) {
    char *source_file = saffire_getopt_string(0);

    setlocale(LC_ALL,"");
    context_init();
    object_init();
    module_init();


    t_bytecode *bc = generate_dummy_bytecode();
    vm_execute(bc);
    return 0;

    t_ast_element *ast = ast_generate_from_file(source_file);
//    char *dest_file = bytecode_generate_destfile(source_file);
//    t_bytecode *bc = bytecode_generate(ast_root, source_file);
//    if (bc && bc->length) {
//        printf("Dumping %d bytes into '%s'\n", bc->length, dest_file);
//        FILE *f = fopen(dest_file, "w+");
//        if (f) {
//            fwrite(bc->buffer, 1, bc->length, f);
//            fclose(f);
//        }
//    }
//
//    bytecode_free(bc);
//    smm_free(dest_file);        // free dest file name

    // Release memory of ast root
    if (ast != NULL) {
        ast_free_node(ast);
    }

    module_fini();
    object_fini();
    context_fini();

    return 0;
}


/****
 * Argument Parsing and action definitions
 ***/


/* Usage string */
static const char help[]  = "Compiles a Saffire script or scripts without running.\n"
                             "\n"
                             "Global settings:\n"
                             "    --sign           Sign the bytecode\n"
                             "    --no-sign        Don't sign the bytecode\n"
                             "    --compress       Compress the bytecode\n"
                             "    --no-compress    Don't compress the bytecode\n"
                             "\n"
                             "If the --[no-]sign and --[no-]compress options aren't given, the bytecode is compressed and signed\n"
                             "according to the configuration settings.\n"
                             "\n"
                             "This command compiles a saffire script or directory into bytecode files.\n";

static void opt_sign(void *data) {
    if (flag_sign > 0) {
        printf("Cannot have both the --no-sign and --sign options");
        exit(1);
    }
    flag_sign = 1;
}
static void opt_no_sign(void *data) {
    if (flag_sign > 0) {
        printf("Cannot have both the --no-sign and --sign options");
        exit(1);
    }
    flag_sign = 2;
}
static void opt_compress(void *data) {
    if (flag_compress > 0) {
        printf("Cannot have both the --no-compress and --compress options");
        exit(1);
    }
    flag_compress = 1;
}
static void opt_no_compress(void *data) {
    if (flag_compress > 0) {
        printf("Cannot have both the --no-compress and --compress options");
        exit(1);
    }
    flag_compress = 2;
}

static struct saffire_option global_options[] = {
    { "sign", "", no_argument, opt_sign },
    { "no-sign", "", no_argument, opt_no_sign },
    { "compress", "", no_argument, opt_compress },
    { "no-compress", "", no_argument, opt_no_compress },
    { 0, 0, 0, 0 }
};


/* Config actions */
static struct command_action command_actions[] = {
    { "", "s", do_compile, NULL },
    { 0, 0, 0, 0 }
};

/* Config info structure */
struct command_info info_compile = {
    "Compiles saffire script",
    command_actions,
    help
};
