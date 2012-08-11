/*
 Copyright (c) 2012, Joshua Thijssen
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
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
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
#include <stdarg.h>
#include "saffire_parser.h"
#include "saffire_compiler.h"
#include "saffire_interpreter.h"
#include "node.h"
#include "parser.tab.h"
#include "svar.h"

extern void yyerror(const char *err);


void saffire_execute(nodeType *p) {
    int mode_compile = 1;
    if (mode_compile) {
        saffire_compiler(p);
    } else {
        saffire_interpreter(p);
    }
}


nodeType *saffire_strCon(char *value) {
    nodeType *p;

    if ((p = malloc(sizeof(nodeType))) == NULL) {
        yyerror("Out of memory");
    }

    p->type = typeStrCon;
    p->strCon.value = strdup(value);

    return p;
}

nodeType *saffire_intCon(int value) {
    nodeType *p;

    if ((p = malloc(sizeof(nodeType))) == NULL) {
        yyerror("Out of memory");
    }

    p->type = typeIntCon;
    p->intCon.value = value;

    return p;
}

nodeType *saffire_var(char *var_name) {
    nodeType *p;

    if ((p = malloc(sizeof(nodeType))) == NULL) {
        yyerror("Out of memory");
    }

    p->type = typeVar;
    p->var.name = strdup(var_name);

    return p;
}

nodeType *saffire_opr(int opr, int nops, ...) {
    va_list ap;
    nodeType *p;

    if ((p = malloc(sizeof(nodeType))) == NULL) {
        yyerror("Out of memory");
    }
    if ((p->opr.ops = malloc (nops * sizeof(nodeType))) == NULL) {
        yyerror("Out of memory");
    }

    p->type = typeOpr;
    p->opr.oper = opr;
    p->opr.nops = nops;

    va_start(ap, nops);
    for (int i =0; i < nops; i++) {
        p->opr.ops[i] = va_arg(ap, nodeType *);
    }
    va_end(ap);

    return p;
}

void saffire_free_node(nodeType *p) {
    if (!p) return;

    // @TODO: If it's a strConOpr, we must free our char as well!
    // @TODO: If it's a varOpr, we must free our svar as well

    if (p->type == typeOpr) {
        for (int i=0; i < p->opr.nops; i++) {
            saffire_free_node(p->opr.ops[i]);
        }
        free(p->opr.ops);
    }
    free(p);
}
