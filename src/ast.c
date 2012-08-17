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
#include <stdarg.h>
#include "parser.tab.h"
#include "svar.h"
#include "ast.h"

extern void yyerror(const char *err);

/**
 *
 */
static t_ast_element *ast_alloc_element(void) {
    t_ast_element *p;

    if ((p = malloc(sizeof(t_ast_element))) == NULL) {
        yyerror("Out of memory");
    }

    return p;
}


/**
 *
 */
t_ast_element *ast_strCon(char *value) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeStrCon;
    p->strCon.value = strdup(value);

    return p;
}


/**
 *
 */
t_ast_element *ast_intCon(int value) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeIntCon;
    p->intCon.value = value;

    return p;
}


/**
 *
 */
t_ast_element *ast_var(char *var_name) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeVar;
    p->var.name = strdup(var_name);

    return p;
}


/**
 *
 */
t_ast_element *ast_nop(void) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeNull;

    return p;
}


/**
 *
 */
t_ast_element *ast_add(t_ast_element *src, t_ast_element *new_element) {
    if (src->type != typeOpr) {
        yyerror("Cannot add to non-opr element");
    }

    src->opr.nops++;
    src->opr.ops = realloc(src->opr.ops, src->opr.nops * sizeof(t_ast_element));
    if (src->opr.ops == NULL) {
        yyerror("Out of memory");
    }
    src->opr.ops[src->opr.nops-1] = new_element;

    return src;
}


/**
 *
 */
t_ast_element *ast_opr(int opr, int nops, ...) {
    t_ast_element *p = ast_alloc_element();
    va_list ap;

    if (nops && (p->opr.ops = malloc (nops * sizeof(t_ast_element))) == NULL) {
        yyerror("Out of memory");
    }

    p->type = typeOpr;
    p->opr.oper = opr;
    p->opr.nops = nops;

    if (nops) {
        va_start(ap, nops);
        for (int i=0; i < nops; i++) {
            p->opr.ops[i] = va_arg(ap, t_ast_element *);
        }
        va_end(ap);
    }

    return p;
}

t_ast_element *ast_class(int modifiers, char *name, t_ast_element *extends, t_ast_element *implements, t_ast_element *body) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeClass;
    p->class.modifiers = modifiers;
    p->class.name = strdup(name);
    p->class.extends = extends;
    p->class.implements = implements;
    p->class.body = body;

    return p;
}

t_ast_element *ast_method(int modifiers, char *name, t_ast_element *arguments, t_ast_element *body) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeMethod;
    p->method.modifiers = modifiers;
    p->method.name = strdup(name);
    p->method.arguments = arguments;
    p->method.body = body;

    return p;
}


/**
 *
 */
void ast_free_node(t_ast_element *p) {
    return;

    if (!p) return;

    // @TODO: If it's a strConOpr, we must free our char as well!
    // @TODO: If it's a varOpr, we must free our svar as well

    if (p->type == typeOpr) {
        for (int i=0; i < p->opr.nops; i++) {
            ast_free_node(p->opr.ops[i]);
        }
        free(p->opr.ops);
    }
    free(p);
}
