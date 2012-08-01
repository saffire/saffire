#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "node.h"
#include "parser.tab.h"
#include "saffire_parser.h"
#include "svar.h"

#include "version.h"

extern int      yyparse();
extern FILE     *yyin;

char    *source_file = "-";     // defaults to stdin
int     source_args = 0;        // default to no additional arguments


/**
 * Prints current version number and copyright information
 */
void print_version() {
    printf("%s  - %s\n", saffire_version, saffire_copyright);
}


/**
 * Prints usage information
 */
void print_usage() {
    printf("\n"
           "Usage: saffire [options] [script [args]]\n"
           "Available options are:\n"
           "  -v, --version    Show version information \n"
           "  -h, --help       This usage information \n"
           "  -c, --cli        Command line interface\n"
           "  -l, --lint FILE  Lint check script\n"
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
            { 0, 0, 0, 0 }
        };

    // Iterate all the options
    while (1) {
        c = getopt_long (argc, argv, "vhcl", long_options, &option_index);
        if (c == -1) break;

        switch (c) {
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
    parse_options(argc, argv);

    // Open file, or use stdin if needed
    FILE *fp = (! strcmp(source_file,"-") ) ? stdin : fopen(source_file, "r");
    if (!fp) {
        fprintf(stderr, "Could not open file: %s\n", source_file);
        return 1;
    }

    // Initialize system
    svar_init_table();

    // Parse the file input
    yyin = fp;
    yyparse();

    return 0;
}
