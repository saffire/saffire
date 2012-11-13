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
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
//#include "interpreter/context.h"
//#include "modules/module_api.h"
//#include "vm/vm.h"
#include "commands/command.h"
#include "general/smm.h"
#include "general/parse_options.h"
#include "general/path_handling.h"
#include "compiler/bytecode.h"

#include "commands/config.h"


int flag_sign = 0;      // 0 = default config setting, 1 = force sign, 2 = force unsigned
int flag_compress = 0;  // 0 = default config setting, 1 = force compress, 2 = force uncompressed


/**
 * Compiles single file
 */
static void _compile_file(const char *source_file, int sign, int compress) {
    char *dest_file = replace_extension(source_file, ".sf", ".sfc");

    printf("Compiling: '%s'\n", source_file);

    t_bytecode *bc = generate_dummy_bytecode();
    save_bytecode_to_disk(dest_file, source_file, bc, sign, compress);

    smm_free(dest_file);
}


/**
 * Scans recursively a directory structure for files with *.sf matching
 */
static void _compile_directory(const char *path, int sign, int compress) {
    DIR *dirp;
    struct dirent *dp;
    char new_path[PATH_MAX];
    int path_length;

    dirp = opendir(path);
    do {
        if ((dp = readdir(dirp)) != NULL) {
            // Explicitly skip "hidden" files
            if (dp->d_name[0] == '.') continue;

            // Add current path to name
            path_length = snprintf(new_path, PATH_MAX, "%s/%s", path, dp->d_name);
            if (path_length >= PATH_MAX) {
                printf("Path too long");
                return;
            }

            if (dp->d_type & DT_DIR) {
                // Found directory. Create new path and recurse
                _compile_directory(new_path, sign, compress);
            } else {
                // Check if file match *.sf
                if (fnmatch("*.sf", dp->d_name, 0) != 0) continue;

                _compile_file(new_path, sign, compress);
            }

        }
    } while (dp);

    closedir(dirp);
}


/**
 *
 */
static int do_compile(void) {
    char *source_path = saffire_getopt_string(0);

    struct stat st;
    if (stat(source_path, &st) != 0) {
        printf("File not found");
        return 1;
    }

    // Get default sign flag and override if needed
    char sign = config_get_bool("compile.sign");
    if (flag_sign == 1) sign = 1;
    if (flag_sign == 2) sign = 0;

    // Get default compress flag and override if needed
    char compress = config_get_bool("compile.compress");
    if (flag_compress == 1) compress = 1;
    if (flag_compress == 2) compress = 0;


    // Compile directory if path matches a directory
    if (S_ISDIR(st.st_mode)) {
        _compile_directory(source_path, sign, compress);
    } else {
        _compile_file(source_path, sign, compress);
    }

    return 0;
}


/****
 * Argument Parsing and action definitions
 ***/


/* Usage string */
static const char help[]  = "Compiles a Saffire script or scripts without running.\n"
                            "\n"
                            "Usage: saffire compile <dir>|<file> [options]\n"
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
                            "This command compiles a saffire script or directory into bytecode files. The output file will be\n"
                            "the source file with a .sfc extension\n";


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
    { "", "s", do_compile, global_options },
    { 0, 0, 0, 0 }
};

/* Config info structure */
struct command_info info_compile = {
    "Compiles saffire script",
    command_actions,
    help
};
