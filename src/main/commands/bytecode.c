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
#include "general/output.h"
#include "vm/vm.h"
#include "vm/frame.h"
#include "commands/command.h"
#include "general/smm.h"
#include "general/parse_options.h"
#include "general/path_handling.h"
#include "compiler/bytecode.h"
#include "objects/numerical.h"
#include "debug.h"

#include "general/config.h"


extern long smm_malloc_calls;
extern long smm_realloc_calls;
extern long smm_strdup_calls;


char *gpg_key = NULL;
int flag_sign = 0;      // 0 = default config setting, 1 = force sign, 2 = force unsigned
int flag_no_verify = 0;

t_bytecode *generate_dummy_bytecode_bc004_bcs_main(void);

/**
 * Compiles single file
 */
static void _compile_file(const char *source_file, int sign, char *gpg_key) {
    char *dest_file = replace_extension(source_file, ".sf", ".sfc");

    output("Compiling %s into %s%s\n", source_file, sign ? "signed " : "", dest_file);

    t_bytecode *bc = generate_dummy_bytecode_bc005_bcs_main();
    bytecode_save(dest_file, source_file, bc);
    bytecode_free(bc);

    // Add signature at the end of the file
    if (sign == 1) {
        bytecode_add_signature(dest_file, gpg_key);
    }

    smm_free(dest_file);
}


/**
 * Scans recursively a directory structure for files with *.sf matching
 */
static void _compile_directory(const char *path, int sign, char *gpg_key) {
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
                error("Path too long");
                return;
            }

            if (dp->d_type & DT_DIR) {
                // Found directory. Create new path and recurse
                _compile_directory(new_path, sign, gpg_key);
            } else {
                // Check if file match *.sf
                if (fnmatch("*.sf", dp->d_name, 0) != 0) continue;

                _compile_file(new_path, sign, gpg_key);
            }

        }
    } while (dp);

    closedir(dirp);
}


/**
 * Add signature to a bytecode file
 */
static int _sign_bytecode(const char *path, char *gpg_key) {
    int ret;

    if (! bytecode_is_valid_file(path)) {
        error("Sign error: This is not a valid saffire bytecode file.\n");
        return 1;
    }

    if (bytecode_is_signed(path)) {
        error("Sign error: This bytecode file is already signed.\n");
        return 1;
    }

    // add signature
    ret = bytecode_add_signature(path, gpg_key);
    if (ret == 0) {
        error("Added signature to bytecode file %s\n", path);
    } else {
        error("Sign error: Error while adding signature from bytecode file %s\n", path);
    }
    return ret;
}


/**
 * Remove signature from a bytecode file
 */
static int _unsign_bytecode(const char *path) {
    int ret;

    if (! bytecode_is_valid_file(path)) {
        error("Sign error: This is not a valid saffire bytecode file.\n");
        return 1;
    }

    if (! bytecode_is_signed(path)) {
        error("Sign error: This bytecode file is not signed.\n");
        return 1;
    }

    // Remove signature
    ret = bytecode_remove_signature(path);
    if (ret == 0) {
        error("Removed signature from bytecode file %s\n", path);
    } else {
        error("Sign error: Error while removing signature from bytecode file %s\n", path);
    }
    return ret;
}


/**
 *
 */
static int do_sign(void) {
    char *source_path = saffire_getopt_string(0);

    struct stat st;
    if (stat(source_path, &st) != 0) {
        error("Cannot sign: File not found\n");
        return 1;
    }

    // sign file
    if (S_ISREG(st.st_mode)) {
        return _sign_bytecode(source_path, gpg_key);
    }

    return 0;
}


/**
 *
 */
static int do_unsign(void) {
    char *source_path = saffire_getopt_string(0);

    struct stat st;
    if (stat(source_path, &st) != 0) {
        error("Cannot sign: File not found\n");
        return 1;
    }

    // Unsign file
    if (S_ISREG(st.st_mode)) {
        return _unsign_bytecode(source_path);
    }

    return 0;
}


/**
 *
 */
static int do_info(void) {
    char *source_path = saffire_getopt_string(0);

    struct stat st;
    if (stat(source_path, &st) != 0) {
        error("File not found\n");
        return 1;
    }

    // sign file
    if (S_ISREG(st.st_mode)) {
        error("Displaying information for bytecode files is not supported yet.");
    }

    return 0;
}


/**
 *
 */
static int do_compile(void) {
    char *source_path = saffire_getopt_string(0);

    struct stat st;
    if (stat(source_path, &st) != 0) {
        error("Cannot compile: file not found\n");
        return 1;
    }

    // Get default sign flag and override if needed
    char sign = config_get_bool("compile.sign", 0);
    if (flag_sign == 1) sign = 1;
    if (flag_sign == 2) sign = 0;

    // Compile directory if path matches a directory
    if (S_ISDIR(st.st_mode)) {
        _compile_directory(source_path, sign, gpg_key);
    } else {
        _compile_file(source_path, sign, gpg_key);
    }

    return 0;
}


/**
 *
 */
static int do_exec(void) {
    struct stat st_src;
    struct stat st_dst;
    char *source_path = saffire_getopt_string(0);

    if (stat(source_path, &st_src) != 0) {
        error("Cannot exec: source file not found\n");
        return 1;
    }

    // Load BC and execute
    char *dest_file = replace_extension(source_path, ".sf", ".sfc");
    DEBUG_PRINT("Loading file: %s\n", dest_file);


    if (stat(dest_file, &st_dst) != 0) {
        error("Cannot exec: bytecode file not found\n");
        return 1;
    }

    int timestamp = bytecode_get_timestamp(dest_file);
    if (timestamp != st_src.st_mtime) {
        output("Warning: Timestamp recorded in bytecode differs from the sourcefile!\n");
    }

    // Get verification flag, and override when we have entered --no-verify flag
    int verify = config_get_bool("bytecode.verify", 1);
    if (flag_no_verify) verify = 0;

    t_bytecode *bc = bytecode_load(dest_file, verify);
    smm_free(dest_file);

    // Init stuff
    setlocale(LC_ALL,"");
    vm_init();

    // Create initial frame
    t_vm_frame *initial_frame = vm_frame_new((t_vm_frame *)NULL, bc);

    t_object *obj1 = vm_execute(initial_frame);

    // Convert returned object to numerical, so we can use it as an error code
    if (! OBJECT_IS_NUMERICAL(obj1)) {
        // Cast to numericak
        t_object *obj2 = object_find_method(obj1, "numerical");
        obj1 = object_call(obj1, obj2, 0);
    }
    int ret = ((t_numerical_object *)obj1)->value;

    vm_frame_destroy(initial_frame);
    bytecode_free(bc);

    // Fini stuff
    vm_fini();

    output("SMM Malloc Calls: %ld\n", smm_malloc_calls);
    output("SMM Realloc Calls: %ld\n", smm_realloc_calls);
    output("SMM Strdup Calls: %ld\n", smm_strdup_calls);

    return (ret & 0xFF);    // Make sure ret is a code between 0 and 255
}



/****
 * Argument Parsing and action definitions
 ***/


/* Usage string */
static const char help[]  = "Compiles a Saffire script or scripts without running.\n"
                            "\n"
                            "Actions:\n"
                            "   compile              Compile saffire script or directory into bytecode\n"
                            "       --sign           Sign the bytecode\n"
                            "       --no-sign        Don't sign the bytecode\n"
                            "   sign                 Sign bytecode file or directory\n"
                            "       --key <key>      Use this key for signing the code\n"
                            "   unsign               Remove signature from bytecode file or directory\n"
                            "   info                 Display information on bytecode file\n"
                            "   exec                 Executes bytecode\n"
                            "\n"
                            "If the --[no-]sign option isn't given, the bytecode is signed according to the configuration settings.\n"
                            "\n";

static void opt_key(void *data) {
    gpg_key = data;
}
static void opt_sign(void *data) {
    if (flag_sign > 0) {
        error_and_die(1, "Cannot have both the --no-sign and --sign options");
    }
    flag_sign = 1;
}
static void opt_no_sign(void *data) {
    if (flag_sign > 0) {
        error_and_die(1, "Cannot have both the --no-sign and --sign options");
    }
    flag_sign = 2;
}
static void opt_no_verify(void *data) {
    flag_no_verify = 1;
}

static struct saffire_option exec_options[] = {
    { "no-verify", "", no_argument, opt_no_verify },
};

static struct saffire_option compile_options[] = {
    { "sign", "", no_argument, opt_sign },
    { "no-sign", "", no_argument, opt_no_sign },
    { "key", "", required_argument, opt_key },
    { 0, 0, 0, 0 }
};

static struct saffire_option sign_options[] = {
    { "key", "", required_argument, opt_key },
    { 0, 0, 0, 0 }
};

/* Config actions */
static struct command_action command_actions[] = {
    { "exec", "s", do_exec, exec_options },
    { "compile", "s", do_compile, compile_options },
    { "sign", "s", do_sign, sign_options },
    { "unsign", "s", do_unsign, NULL },
    { "info", "s", do_info, NULL },
    { 0, 0, 0, 0 }
};

/* Config info structure */
struct command_info info_bytecode = {
    "Compiles saffire script",
    command_actions,
    help
};
