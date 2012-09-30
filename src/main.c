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

#include "version.h"

char    *source_file = "-";     // defaults to stdin
int     source_args = 0;        // default to no additional arguments
int     generate_dot = 0;
char    *dot_file = NULL;

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
           "Usage: saffire [options] [script [args]]\n"
           "Available options are:\n"
           "  -v, --version         Show version information \n"
           "  -h, --help            This usage information \n"
           "  -c, --cli             Command line interface\n"
           "  -l, --lint            Lint check script\n"
           "      --dot <output>    Generate an AST in .dot format\n"
           "\n"
           "With no FILE, or FILE is -, read standard input.\n"
           "\n");
}


/**
 * parses options and set some (global) variables if needed
 */
void parse_options(int argc, char *argv[]) {
    int c;
    int option_index;

    // Suppress default errors
    opterr = 0;

    // Long options maps back to short options
    static struct option long_options[] = {
            { "version", no_argument, 0, 'v' },
            { "help",    no_argument, 0, 'h' },
            { "cli",     no_argument, 0, 'c' },
            { "lint",    no_argument, 0, 'l' },
            { "dot",     required_argument, 0, 0 },
            { 0, 0, 0, 0 }
        };

    // Iterate all the options
    while (1) {
        c = getopt_long (argc, argv, "vhcl", long_options, &option_index);
        if (c == -1) break;

        switch (c) {
            case 0 :
                // Long option without any short alias is called. Have to strcmp the string itself
                if (strcmp(long_options[option_index].name, "dot") == 0) {
                    generate_dot = 1;
                    dot_file = optarg;
                    break;
                }
            case 'h' :
                print_version();
                print_usage();
                exit(0);
                break;
            case 'v' :
                print_version();
                exit(0);
                break;
            case 'l' :
                printf("Lint check()");
                break;
            case 'c' :
                printf("cli");
                break;

            case '?' :
            default :
                printf("saffire: invalid option '%s'\n"
                       "Try 'saffire --help' for more information\n", argv[optind-1]);
                exit(1);
        }
    }

    // All options done, check for additional options like source filename and optional arguments
    if (optind < argc) {
        source_file = argv[optind++];
        if (optind < argc) {
            // Source args points to the FIRST saffire script argument (./saffire script.sf first second third)
            source_args = optind;
        }
    }
}


int main(int argc, char *argv[]) {
    setlocale(LC_ALL,"");
    object_init();


    parse_options(argc, argv);

    // Open file, or use stdin if needed
    FILE *fp = (! strcmp(source_file,"-") ) ? stdin : fopen(source_file, "r");
    if (!fp) {
        fprintf(stderr, "Could not open file: %s\n", source_file);
        return 1;
    }

    // Compile file into a tree
    ast_compile_tree(fp);

    // Close file
    fclose(fp);

    if (generate_dot) {
        // generate DOT file
        dot_generate(ast_root, dot_file);
    } else {
        // Otherwise interpret it
        saffire_interpreter(ast_root);
    }

    // Release memory of ast_root
    if (ast_root != NULL) {
        ast_free_node(ast_root);
    }

    return 0;
}