#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "node.h"
#include "parser.tab.h"
#include "saffire_parser.h"

#include "svar.h"

extern int yyparse();
extern FILE *yyin;


int main(int argc, char *argv[]) {
    svar_init_table();

    // Usage
    if (argc < 2) {
        fprintf(stderr, "Please specify source file\n");
        return 1;
    }

    // Open file
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Could not open file: %s\n", argv[1]);
        return 1;
    }

    // Parse it
    yyin = fp;
    yyparse();

    return 0;
}
