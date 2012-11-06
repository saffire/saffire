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
#include <stdlib.h>
#include <string.h>
#include <histedit.h>

#define HIST_FILE ".saffire_history"

#define MAX_PROMPT_SIZE 40
char cur_prompt[MAX_PROMPT_SIZE];

static int statement_count = 1;

// @TODO: saffire.prompt will actually set the current prompt.. saffire.prompt("%n>");  %n == current stament count?

char *prompt(EditLine *el) {
    snprintf(cur_prompt, MAX_PROMPT_SIZE-1, "%d> ", statement_count++);
    return cur_prompt;
}

int repl(void) {
    EditLine *el;
    History *hist;
    HistEvent ev;
    const char *line;
    int count;

    printf("Interactive/CLI mode is not yet supported. However, you can type text here.. use CTRL-C to quit.\n");

    // initialize EditLine library
    el = el_init("saffire", stdin, stdout, stderr);
    el_set(el, EL_PROMPT, &prompt);
    el_set(el, EL_EDITOR, "emacs");

    // Initialize history
    hist = history_init();
    if (! hist) {
        fprintf(stderr, "Warning: cannot initialize history\n");
    }
    history(hist, &ev, H_SETSIZE, 800);
    el_set(el, EL_HIST, history, hist);

    // Load history file
    history(hist, &ev, H_LOAD, HIST_FILE);

    while (1) {
        const char *line = el_gets(el, &count);
        if (count > 0) {
            history(hist, &ev, H_ENTER, line);
            history(hist, &ev, H_SAVE, HIST_FILE);
//            printf("LINE: %s", line);
        }
    }

    history_end(hist);
    el_end(el);

    return 0;
}
