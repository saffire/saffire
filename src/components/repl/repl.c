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
#include <stdlib.h>
#include <string.h>
#include <histedit.h>
#include <signal.h>
#include "compiler/saffire_parser.h"
#include "compiler/parser.tab.h"
#include "compiler/lex.yy.h"
#include "general/config.h"
#include "repl/repl.h"
#include "general/output.h"
#include "version.h"
#include "compiler/output/asm.h"
#include "compiler/ast_to_asm.h"
#include "vm/vm.h"

const char *repl_logo = "   _____        ,__  ,__                \n"
                        "  (        ___  /  ` /  ` ` .___    ___ \n"
                        "   `--.   /   ` |__  |__  | /   \\ .'   `\n"
                        "      |  |    | |    |    | |   ' |----'\n"
                        " \\___.'  `.__/| |    |    / /     `.___,\n"
                        "                /    /                  \n"
                        "\n"
                        saffire_version " interactive/REPL mode. Use CTRL-D to quit.\n";

// Forward defines
char *repl_prompt(EditLine *el);
int yyparse (yyscan_t scanner, SaffireParser *saffireParser);


// Default history file
#define DEFAULT_HIST_FILE ".saffire_history"

// Actual history file
char *hist_file = DEFAULT_HIST_FILE;


/**
 * Callback function that is called by yyparse whenever it needs data
 */
int repl_readline(void *_as, int lineno, char *buf, int max) {
    repl_argstruct_t *as = (repl_argstruct_t *)_as;

    // We need to set the linenumber inside our structure, otherwise prompt() does not know the correct linenumber
    as->lineno = lineno;

    // Let editline get a bit of input from the user.
    int count;
    const char *line = el_gets(as->el, &count);
    if (count > 0) {
        history(as->hist, &as->ev, H_SAVE, hist_file);
        history(as->hist, &as->ev, H_ENTER, line);
    }

    // Make sure we don't overflow our 'buf'.
    // @TODO: What to do with the remaining characters???
    if (count > max) count = max;

    strncpy(buf, line, count);
    return count;
}


/**
 * Callback function that is called by yyexec whenever parsed data has been converted to an AST
 */
void repl_exec(SaffireParser *sp) {
    if (! sp->ast) {
        printf("\033[34;1m");
        printf("No repl_exec needed, as there is no ast right now.\n");
    } else {
        printf("\033[32;1m");
        printf("---- repl_exec ------------------------------------------------\n");

        t_hash_table *asm_code = ast_to_asm(sp->ast, 0);
        assembler_output_stream(asm_code, stdout);
        t_bytecode *bc = assembler(asm_code, NULL);
        bc = 0;
        //vm_execute(sp->initial_frame, bc);

        printf("---------------------------------------------------------------\n");
    }
    printf("\033[0m");
    printf("\n");

    repl_argstruct_t *args = (repl_argstruct_t *)sp->yyparse_args;
    args->atStart = 1;
}



/**
 * Main repl function
 */
int repl(void) {
    // Ignore CTRL-C. We use CTRL-D to represent EOF / exit
    signal(SIGINT, SIG_IGN);

    // @TODO: remove configuration reading like this
    config_init("/etc/saffire/saffire.ini");

    /*
     * Initialize our repl argument structure
     */
    repl_argstruct_t repl_as;

    repl_as.ps1 = config_get_string("repl.ps1", ">");
    repl_as.ps2 = config_get_string("repl.ps2", "...>");
    repl_as.context = "global";
    repl_as.completeLine = 0;
    repl_as.atStart = 1;
    repl_as.echo = NULL;


    // initialize EditLine library
    repl_as.el = el_init("saffire", stdin, stdout, stderr);
    el_set(repl_as.el, EL_PROMPT, &repl_prompt);
    el_set(repl_as.el, EL_EDITOR, config_get_string("repl.editor", "emacs"));


    // Initialize history
    repl_as.hist = history_init();
    if (! repl_as.hist) {
        fprintf(stderr, "Warning: cannot initialize history\n");
    }
    history(repl_as.hist, &repl_as.ev, H_SETSIZE, config_get_long("repl.history.size", 800));
    el_set(repl_as.el, EL_HIST, history, repl_as.hist);

    // Load history file
    hist_file = config_get_string("repl.history.path", DEFAULT_HIST_FILE);
    history(repl_as.hist, &repl_as.ev, H_LOAD, hist_file);


    /*
     * Init parser structures
     */
    SaffireParser *sp = (SaffireParser *)malloc(sizeof(SaffireParser));
    yyscan_t scanner;

    // Initialize saffire structure
    sp->mode = SAFFIRE_EXECMODE_REPL;        // @todo we should get vm_runmode in sync with this
    sp->file = NULL;
    sp->filename = "<console>";
    sp->eof = 0;
    sp->ast = NULL;
    sp->error = NULL;
    sp->yyparse = repl_readline;
    sp->yyparse_args = (void *)&repl_as;
    sp->yyexec = repl_exec;
    sp->parserinfo = alloc_parserinfo();

    // Initialize scanner structure and hook the saffire structure as extra info
    yylex_init_extra(sp, &scanner);

    // We need to link our scanner into the editline, so we can use it inside the repl_prompt() function
    el_set(repl_as.el, EL_CLIENTDATA, scanner);


    // Display the logo if needed
    if (config_get_bool("repl.logo", 1) == 1) {
        output(repl_logo);
    }

    // Initialize runner
    t_vm_frame *initial_frame = vm_init(sp, VM_RUNMODE_REPL);

    // Main loop of the REPL
    while (! sp->eof) {
        // New 'parse' loop
        repl_as.atStart = 1;
        int status = yyparse(scanner, sp);

        printf("Repl: yyparse() returned %d\n", status);

        // Did something went wrong?
        if (status) {
            if (sp->error) {
                fprintf(stdout, "Error: %s\n", sp->error);
                free(sp->error);
            }
            continue;
        }

        // Do something with our data

        if (sp->mode == SAFFIRE_EXECMODE_REPL && repl_as.echo != NULL)  {
            printf("repl output: '%s'\n", repl_as.echo);
            free(repl_as.echo);
            repl_as.echo = NULL;
        }
    }


    /*
     * Here be generic finalization
     */
    free_parserinfo(sp->parserinfo);

    vm_fini(initial_frame);

    // Destroy scanner structure
    yylex_destroy(scanner);

    // Destroy everything else
    history_end(repl_as.hist);
    el_end(repl_as.el);

    return 0;
}
