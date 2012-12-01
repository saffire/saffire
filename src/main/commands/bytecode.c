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
#include "dot/dot.h"
#include "compiler/ast.h"
#include "compiler/ast_walker.h"
#include "compiler/assembler.h"

extern char *vm_code_names[];
extern int vm_codes_index[];
extern int vm_codes_offset[];

int write_dot = 0;                  // 1 = write DOT file
int write_sfa = 0;                  // 1 = write saffire assembly file
char *forced_gpg_key = NULL;        // When set, overrides the configuration gpg key
int flag_sign = 0;                  // 0 = default config setting, 1 = force sign, 2 = force unsigned

/**
 * Compiles single file
 */
static int _compile_file(const char *source_file, int sign, char *gpg_key) {
    char *sfc_dest_file = NULL;
    char *sfa_dest_file = NULL;
    char *dot_dest_file = NULL;
    t_ast_element *ast = NULL;
    t_dll *asm_code = NULL;
    int ret = 0;

    // Convert our saffire source to an AST
    output("SRC -> AST\n");
    ast = ast_generate_from_file(source_file);
    if (! ast) {
        ret = 1;
        goto cleanup;
    }

    // Write dot output file if needed
    if (write_dot) {
        dot_dest_file = replace_extension(source_file, ".sf", "dot");
        dot_generate(ast, dot_dest_file);
    }

    // Convert the AST to assembler lines
    output("AST -> ASM\n");
    asm_code = ast_walker(ast);
    if (! asm_code) {
        ret = 1;
        goto cleanup;
    }

    // Write assembly output file if needed
    if (write_sfa) {
        char *sfa_dest_file = replace_extension(source_file, ".sf", ".sfa");
        assembler_output(asm_code, sfa_dest_file);
    }

    // Convert the assembler lines to bytecode
    output("ASM -> BC\n");
    t_bytecode *bc = assembler(asm_code);
    if (! bc) {
        ret = 1;
        goto cleanup;
    }

    // Save bytecode structure to disk
    output("BC -> disk\n");
    sfc_dest_file = replace_extension(source_file, ".sf", ".sfc");
    output("Compiling %s into %s%s\n", source_file, sign ? "signed " : "", sfc_dest_file);
    bytecode_save(sfc_dest_file, source_file, bc);

    // Add signature at the end of the file, if needed
    if (sign == 1) {
        bytecode_add_signature(sfc_dest_file, gpg_key);
    }

cleanup:
    if (ast) ast_free_node(ast);
    if (sfc_dest_file) smm_free(sfc_dest_file);
    if (dot_dest_file) smm_free(dot_dest_file);
    if (sfa_dest_file) smm_free(sfa_dest_file);
    if (asm_code) assembler_free(asm_code);

    return ret;
}

/**
 * Scans recursively a directory structure for files with *.sf matching
 */
static int _compile_directory(const char *path, int sign, char *gpg_key) {
    DIR *dirp;
    struct dirent *dp;
    char new_path[PATH_MAX];
    int path_length;
    int ret = 0;

    // Read complete directory
    dirp = opendir(path);
    do {
        if ((dp = readdir(dirp)) != NULL) {
            // Explicitly skip "hidden" files
            if (dp->d_name[0] == '.') continue;

            // Add current path to name
            path_length = snprintf(new_path, PATH_MAX, "%s/%s", path, dp->d_name);
            if (path_length >= PATH_MAX) {
                error("Path too long");
                return 1;
            }

            if (dp->d_type & DT_DIR) {
                // Found directory. Create new path and recurse
                ret += _compile_directory(new_path, sign, gpg_key);
            } else {
                // Check if file match *.sf
                if (fnmatch("*.sf", dp->d_name, 0) != 0) continue;

                ret += _compile_file(new_path, sign, gpg_key);
            }

        }
    } while (dp);

    closedir(dirp);

    return (ret > 0) ? 1 : 0;
}

/**
 * Add signature to a bytecode file
 */
static int _sign_bytecode(const char *path, char *gpg_key) {
    int ret;

    if (!bytecode_is_valid_file(path)) {
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

    if (!bytecode_is_valid_file(path)) {
        error("Sign error: This is not a valid saffire bytecode file.\n");
        return 1;
    }

    if (!bytecode_is_signed(path)) {
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
        char *gpg_key = config_get_string("gpg.key", forced_gpg_key);
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
    int ret = 0;

    struct stat st;
    if (stat(source_path, &st) != 0) {
        error("Cannot compile: file not found\n");
        return 1;
    }

    // Get default sign flag and override if needed
    char sign = config_get_bool("compile.sign", 0);
    if (flag_sign == 1) sign = 1;
    if (flag_sign == 2) sign = 0;

    char *gpg_key = config_get_string("gpg.key", forced_gpg_key);

    // Compile directory if path matches a directory
    if (S_ISDIR(st.st_mode)) {
        ret = _compile_directory(source_path, sign, gpg_key);
    } else {
        ret = _compile_file(source_path, sign, gpg_key);
    }

    return ret;
}




/****
 * Argument Parsing and action definitions
 ***/


/* Usage string */
static const char help[] = "Compiles a Saffire script or scripts without running.\n"
    "\n"
    "Actions:\n"
    "   compile              Compile saffire script or directory into bytecode (as filename.sfc)\n"
    "       --text           Generate textual assembly output (as filename.sfa)\n"
    "       --dot            Generate DOT output fromt the AST (as filename.dot)\n"
    "       --sign           Sign the bytecode\n"
    "       --no-sign        Don't sign the bytecode\n"
    "       --key <key>      Use this key for signing the code\n"
    "   sign                 Sign bytecode file or directory\n"
    "       --key <key>      Use this key for signing the code\n"
    "   unsign               Remove signature from bytecode file or directory\n"
    "   info                 Display information on bytecode file\n"
    "\n"
    "If the --[no-]sign option isn't given, the bytecode is signed according to the configuration settings.\n"
    "\n";

static void opt_text(void *data) {
    write_sfa = 1;
}
static void opt_dot(void *data) {
    write_dot = 1;
}


static void opt_key(void *data) {
    forced_gpg_key = data;
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


static struct saffire_option compile_options[] = {
    { "sign", "", no_argument, opt_sign},
    { "no-sign", "", no_argument, opt_no_sign},
    { "key", "", required_argument, opt_key},
    { "text", "", no_argument, opt_text},
    { "dot", "", no_argument, opt_dot},
    { 0, 0, 0, 0}
};

static struct saffire_option sign_options[] = {
    { "key", "", required_argument, opt_key},
    { 0, 0, 0, 0}
};

/* Config actions */
static struct command_action command_actions[] = {
    { "compile", "s", do_compile, compile_options},
    { "sign", "s", do_sign, sign_options},
    { "unsign", "s", do_unsign, NULL},
    { "info", "s", do_info, NULL},
    { 0, 0, 0, 0}
};

/* Config info structure */
struct command_info info_bytecode = {
    "Compiles saffire script",
    command_actions,
    help
};
