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
#include "compiler/saffire_parser.h"
#include "compiler/parser.tab.h"
#include "compiler/lex.yy.h"

int yyparse (yyscan_t scanner, SaffireParser *saffireParser);

typedef struct repl_argstruct {
    EditLine    *el;
    History     *hist;
    HistEvent   ev;

    char        *ps1;           // prompt to start statement
    char        *ps2;           // prompt to continue statement
    char        *context;       // prompt for context
    int         lineno;         // Current Line number


    int         atStart;        // True before scanner sees printable chars on line
    char        *echo;          // result of last statement to display
    int         completeLine;   // Managed by yyread
} repl_argstruct_t;


#define HIST_FILE ".saffire_history"


#define MAX_PROMPT_SIZE 100
char cur_prompt[MAX_PROMPT_SIZE+1];

/**
 *
 */
char *prompt(EditLine *el) {
    yyscan_t scanner;
    el_get(el, EL_CLIENTDATA, (yyscan_t)&scanner);

    SaffireParser *sp = (SaffireParser *)yyget_extra(scanner);
    repl_argstruct_t *args = (repl_argstruct_t *)sp->yyparse_args;

    snprintf(cur_prompt, MAX_PROMPT_SIZE-1, "(\033[32m%d\033[0m) [\033[33m%s\033[0m] %s", args->lineno, args->context, args->atStart ? args->ps1 : args->ps2);
    return cur_prompt;
}

int repl_readline(void *_as, int lineno, char *buf, int max) {
    repl_argstruct_t *as = (repl_argstruct_t *)_as;

    // We need to set the linenumber inside our structure, otherwise prompt() does not know the correct linenumber
    as->lineno = lineno;

    int count;
    const char *line = el_gets(as->el, &count);
    if (count > 0) {
        history(as->hist, &as->ev, H_SAVE, HIST_FILE);
        history(as->hist, &as->ev, H_ENTER, line);
    }

    // Make sure we don't overflow our 'buf'.
    // @TODO: What to do with the remaining characters???
    if (count > max) count = max;

    strncpy(buf, line, count);
    return count;
}

int repl(void) {
    /*
     * Init history and setup repl argument structure
     */
    repl_argstruct_t repl_as;

    repl_as.ps1 = strdup(">");
    repl_as.ps2 = strdup("...>");
    repl_as.context = strdup("global");
    repl_as.completeLine = 0;
    repl_as.atStart = 1;
    repl_as.echo = NULL;


    // initialize EditLine library
    repl_as.el = el_init("saffire", stdin, stdout, stderr);
    el_set(repl_as.el, EL_PROMPT, &prompt);
    el_set(repl_as.el, EL_EDITOR, "emacs");

    // Initialize history
    repl_as.hist = history_init();
    if (! repl_as.hist) {
        fprintf(stderr, "Warning: cannot initialize history\n");
    }
    history(repl_as.hist, &repl_as.ev, H_SETSIZE, 800);
    el_set(repl_as.el, EL_HIST, history, repl_as.hist);

    // Load history file
    history(repl_as.hist, &repl_as.ev, H_LOAD, HIST_FILE);


    /*
     * Init parser structures
     */
    SaffireParser sp;
    yyscan_t scanner;

    // Initialize saffire structure
    sp.mode = SAFFIRE_EXECMODE_REPL;
    sp.filehandle = NULL;
    sp.eof = 0;
    sp.ast = NULL;
    sp.error = NULL;
    sp.yyparse = repl_readline;
    sp.yyparse_args = (void *)&repl_as;

    // Initialize scanner structure and hook the saffire structure as extra info
    yylex_init_extra(&sp, &scanner);

    // We need to link our scanner into the editline, so we can use it inside the prompt() function
    el_set(repl_as.el, EL_CLIENTDATA, scanner);

    printf("Saffire interactive/REPL mode. Use CTRL-C to quit.\n");


    /*
     * Global initialization
     */
    parser_init();

    // Mainloop
    while (! sp.eof) {
        // New 'parse' loop
        repl_as.atStart = 1;
        int status = yyparse(scanner, &sp);
        printf("Returning from yyparse() with status %d\n", status);

        // Did something went wrong?
        if (status) {
            if (sp.error) {
                fprintf(stdout, "Error: %s\n", sp.error);
                free(sp.error);
            }
            continue;
        }

        // Do something with our data

        if (sp.mode == SAFFIRE_EXECMODE_REPL && repl_as.echo != NULL)  {
            printf("repl output: '%s'\n", repl_as.echo);
            free(repl_as.echo);
            repl_as.echo = NULL;
        }
    }

    // Here be generic finalization
    parser_fini();

    // Destroy scanner structure
    yylex_destroy(scanner);

    history_end(repl_as.hist);
    el_end(repl_as.el);

    return 0;
}
