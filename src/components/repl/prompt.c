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
#include <time.h>
#include <histedit.h>
#include "repl/repl.h"
#include "compiler/saffire_parser.h"
#include "compiler/parser.tab.h"
#include "compiler/lex.yy.h"

// Maximum size of the prompt that will be displayed
#define MAX_PROMPT_SIZE 100

// Actual prompt.
char cur_prompt[MAX_PROMPT_SIZE+1];

/**
 * @todo: move to a more generic part in general/
 */
int str_append_char(char *buf, long buf_len, char c) {
    int bl = strlen(buf);

    // No more room to add the character
    if (bl == buf_len) return 0;

    // Add character and trailing zero
    buf[bl+1] = '\0';
    buf[bl] = c;
    return 1;
}

/**
 * @todo: move to a more generic part in general/
 */
int str_append_str(char *buf, long buf_len, char *s) {
    int ret = 1;

    // Check if the string actually fits
    if (strlen(buf) + strlen(s) > buf_len) {
        // Cut down s so it will fit
        int cutpoint = strlen(buf) + strlen(s) - buf_len;
        s[cutpoint] = '\0';

        ret = 0;
    }

    strcat(buf, s);
    return ret;
}


/**
 * Expands the placeholders inside a given prompt.
 */
static void repl_convert_prompt(char *buf, int buf_len, repl_argstruct_t *args) {
    char *p = args->atStart ? args->ps1 : args->ps2;

    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);

    // Clear buf
    strcpy(buf, "");


    int i = 0;
    int state = 0;
    for (i=0; i!=strlen(p); i++) {
        // Just add character when it's a normal one
        if (state == 0 && p[i] != '%') {
            str_append_char(buf, buf_len, p[i]);
            continue;
        }

        if (p[i] == '%') {
            if (state == 0) {
                // initial % found
                state = 1;

                // Make sure we skip the next parts. Just fetch new character
                continue;
            }
            if (state == 1) {
                // Second % found
                str_append_char(buf, buf_len, p[i]);

                state = 0;
            }
        }
        if (state == 1) {
            if (p[i] == 'a') {
                // ansi escape code
                str_append_char(buf, buf_len, 0x1B);
            }
            if (p[i] == 'd') {
                // dd/mm/yyyy date
                char tmpbuf[100];
                strftime(tmpbuf, 100, "%m/%d/%Y", timeinfo);
                str_append_str(buf, buf_len, tmpbuf);
            }
            if (p[i] == 'D') {
                // mm/dd/yyyy date
                char tmpbuf[100];
                strftime(tmpbuf, 100, "%d/%m/%Y", timeinfo);
                str_append_str(buf, buf_len, tmpbuf);
            }
            if (p[i] == 'l') {
                // Line number
                char tmpbuf[100];
                snprintf(tmpbuf, 100, "%d", args->lineno);
                str_append_str(buf, buf_len, tmpbuf);
            }
            if (p[i] == 'n') {
                // Newline
                str_append_char(buf, buf_len, '\n');
            }
            if (p[i] == 'x') {
                // Context
                str_append_str(buf, buf_len, args->context);
            }
            if (p[i] == 't') {
                // h:i:s time
                char tmpbuf[100];
                strftime(tmpbuf, 100, "%I:%M:%S", timeinfo);
                str_append_str(buf, buf_len, tmpbuf);
            }
            if (p[i] == 'T') {
                // H:i:s time
                char tmpbuf[100];
                strftime(tmpbuf, 100, "%H:%M:%S", timeinfo);
                str_append_str(buf, buf_len, tmpbuf);
            }

            state = 0;
        }
    }

}

/**
 * Returns the prompt to use by editline
 */
char *repl_prompt(EditLine *el) {
    yyscan_t scanner;
    el_get(el, EL_CLIENTDATA, (yyscan_t)&scanner);

    SaffireParser *sp = (SaffireParser *)yyget_extra(scanner);
    repl_argstruct_t *args = (repl_argstruct_t *)sp->yyparse_args;

    // Generate a complete prompt
    repl_convert_prompt(cur_prompt, MAX_PROMPT_SIZE-1, args);
    return cur_prompt;
}
