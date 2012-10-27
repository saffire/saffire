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
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <locale.h>
#include "dot/dot.h"
#include "compiler/ast.h"
#include "general/smm.h"
#include "object/object.h"
#include "interpreter/saffire_interpreter.h"
#include "interactive/interactive.h"
#include "modules/module_api.h"
#include "interpreter/context.h"
#include "general/levenshtein.h"
#include "version.h"

int cmd_help(int argc, const char **argv);
int cmd_version(int argc, const char **argv);
int cmd_lint(int argc, const char **argv);
int cmd_fastcgi(int argc, const char **argv);
int cmd_config(int argc, const char **argv);


#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

struct _command {
    char *name;
    char *description;
    int (*func)(int, const char **);
    int option;
};

static struct _command commands[] = {
    { "help", "Displays help information", cmd_help, 0 },
    { "version", "Displays version", cmd_version, 0 },
    { "lint", "Lint check a file", cmd_lint, 0 },
    { "fastcgi", "Run the FastCGI server", cmd_fastcgi, 0 },
    { "config", "Display (or generate) new configuration settings", cmd_config, 0 },
};

/**
 * Prints current version number and copyright information
 */
void print_version() {
    printf("%s  - %s\n%s\n", saffire_version, saffire_copyright, saffire_compiled);
}

/**
 * Prints usage information
 */
void print_usage() {
    printf("\n"
           "Usage: saffire <command> [options] [script [args]]\n"
           "Available options are:\n"
           "  -v, --version         Show version information \n"
           "  -h, --help            This usage information \n"
           "      --dot <output>    Generate an AST in .dot format\n"
           "\n"
           "Available commands:\n");

    for (int i=0; i<ARRAY_SIZE(commands); i++) {
        struct _command *p = commands+i;
        printf("%-15s %s\n", p->name, p->description);
    }
    printf("\n");
}

///**
// * parses options and set some (global) variables if needed
// */
//void parse_options(int argc, char *argv[]) {
//    int c;
//    int option_index;
//
//    // Suppress default errors
//    opterr = 0;
//
//    // Long options maps back to short options
//    static struct option long_options[] = {
//            { "version", no_argument, 0, 'v' },
//            { "help",    no_argument, 0, 'h' },
//            { "lint",    no_argument, 0, 'l' },
//            { "dot",     required_argument, 0, 0 },
//            { 0, 0, 0, 0 }
//        };
//
//    // Iterate all the options
//    while (1) {
//        c = getopt_long (argc, argv, "vhl", long_options, &option_index);
//        if (c == -1) break;
//
//        switch (c) {
//            case 0 :
//                // Long option without any short alias is called. Have to strcmp the string itself
//                if (strcmp(long_options[option_index].name, "dot") == 0) {
//                    op_mode |= OPMODE_DOTFILE;
//                    dot_file = optarg;
//                    break;
//                }
//            case 'h' :
//                print_version();
//                print_usage();
//                exit(0);
//                break;
//            case 'v' :
//                print_version();
//                exit(0);
//                break;
//            case 'l' :
//                op_mode |= OPMODE_LINT;
//                break;
//
//            case '?' :
//            default :
//                printf("saffire: invalid option '%s'\n"
//                       "Try 'saffire --help' for more information\n", argv[optind-1]);
//                exit(1);
//        }
//    }
//
//    // All options done, check for additional options like source filename and optional arguments
//    if (optind < argc) {
//        op_mode &= ~OPMODE_CLI;
//        source_file = argv[optind++];
//        if (optind < argc) {
//            // Source args points to the FIRST saffire script argument (./saffire script.sf first second third)
//            saffire_argc = optind;
//        }
//    }
//}

int cmd_help(int argc, const char **argv) {
    print_usage();
    return 0;
}

int cmd_version(int argc, const char **argv) {
    print_version();
    return 0;
}
int cmd_lint(int argc, const char **argv) {
    printf("lint check!");
    return 0;
}
int cmd_fastcgi(int argc, const char **argv) {
    printf("FastCGI mode!");
    return 0;
}
int cmd_config(int argc, const char **argv) {
    // saffire config set x.y.z 12345
    // saffire config get x.y.z
    // saffire config get x
    // saffire config generate > config.ini
    return 0;
}


int exec_command(struct _command *cmd, int argc, const char **argv) {
    printf("Executing: %s\n", cmd->name);
    return cmd->func(argc, argv);
}


int main(int argc, char *argv[]) {
    char *cmd = argv[1];
    int ret = -1;

    // Turn "saffire cmd --help" into "saffire help cmd"
    if (argc > 2 && ! strcmp(argv[2], "--help")) {
        argv[2] = argv[0];
        argv[1] = cmd = "help";
    }

    // Iterate all commands, see if we have a match and run it.
    for (int i=0; i<ARRAY_SIZE(commands); i++) {
        struct _command *p = commands+i;
        if (strcmp(p->name, cmd) != 0) continue;
        ret = exec_command(p, argc, (const char **)argv);
    }

    // Did not found or execute a command. Give some help.
    if (ret == -1) {
        // Do a levenshtein check to see if we might mistyped a command or so
        int first = 1;
        for (int i=0; i<ARRAY_SIZE(commands); i++) {
            struct _command *p = commands+i;
            if (levenshtein(p->name, argv[1]) < 4) {
                if (first) {
                    printf("Did you mean on of the following commands: \n");
                    printf("\n");
                }
                first = 0;
                printf("%10s  : %s\n", p->name, p->description);
            }
        }
    }
    return ret;
}



//  Init stuff:
//    setlocale(LC_ALL,"");
//    context_init();
//    object_init();
//    module_init();
//
//    parse_options(argc, argv);
//
//    // Opmode CLI is easy enough
//    if ((op_mode & OPMODE_CLI) == OPMODE_CLI) {
//        print_version();
//        interactive();
//        return 0;
//    }
//
//    // Otherwise we can run from a file
//    ast_root = ast_generate_from_file(source_file);
//
//    if ((op_mode & OPMODE_DOTFILE) == OPMODE_DOTFILE) {
//        // generate DOT file
//        dot_generate(ast_root, dot_file);
//    }
//    if ((op_mode & OPMODE_LINT) == OPMODE_LINT) {
//        // Do nothing for lint check. It's ok, since there is an AST
//        printf ("Syntax OK\n");
//    } else {
//        // Otherwise, just interpret it
//        saffire_interpreter(ast_root);
//    }
//

//  Fini Stuff:
//    // Release memory of ast_root
//    if (ast_root != NULL) {
//        ast_free_node(ast_root);
//    }
//
//    module_fini();
//    object_fini();
//    context_fini();
//    return 0;
//}