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
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include "objects/object.h"
#include "modules/module_api.h"
#include "compiler/ast_nodes.h"
#include "general/smm.h"
#include "commands/command.h"
#include "general/parse_options.h"
#include "general/path_handling.h"
#include "vm/vm.h"
#include "general/output.h"
#include "debug.h"

static int flag_debug = 0;
static int flag_no_verify = 0;
int write_bytecode = 1;


static int do_exec(void) {
    char *source_file = saffire_getopt_string(0);
    struct stat source_stat;
    t_bytecode *bc;

    // Check if sourcefile exists
    if (stat(source_file, &source_stat) != 0) {
        warning("Source file '%s' does not exist.\n", source_file);
        return 1;
    }

    char full_source_path[PATH_MAX+1];
    char *ptr;
    ptr = realpath(source_file, full_source_path);

    // Check if bytecode exists, or has a correct timestamp
    char *bytecode_file = replace_extension(source_file, ".sf", ".sfc");
    int bytecode_exists = (access(bytecode_file, F_OK) == 0);

    if (! bytecode_exists || bytecode_get_timestamp(bytecode_file) != source_stat.st_mtime) {
        bc = bytecode_generate_diskfile(source_file, write_bytecode ? bytecode_file : NULL, NULL);
    } else {
        bc = bytecode_load(bytecode_file, flag_no_verify);
    }
    smm_free(bytecode_file);

    // Something went wrong with the bytecode loading or generating
    if (!bc) {
        warning("Error while loading bytecode\n");
        return 1;
    }

    // Create initial frame and attach our bytecode to it
    int runmode = VM_RUNMODE_CLI;
    if (flag_debug) runmode |= VM_RUNMODE_DEBUG;
    t_vm_frame *initial_frame = vm_init(NULL, runmode);
    vm_attach_bytecode(initial_frame, "{main}", bc);

    // Run the frame
    int exitcode = vm_execute(initial_frame);
    vm_fini(initial_frame);

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
                             "   --debug                Start debugger connection\n"
                             "   --no-verify            Don't verify signature from bytecode file (if any)\n"
                             "   --no-write-bytecode    Don't write bytecode to disk\n"
                             "\n"
                             "This command executes a saffire script or bytecode file.\n";


static void opt_debug(void *data) {
    flag_debug = 1;
}

static void opt_no_verify(void *data) {
    flag_no_verify = 1;
}

static void opt_no_write_bytecode(void *data) {
    write_bytecode = 0;
}

static struct saffire_option exec_options[] = {
    { "no-verify", "", no_argument, opt_no_verify},
    { "no-write-bytecode", "", no_argument, opt_no_write_bytecode},
    { "debug", "", no_argument, opt_debug }
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
