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
#include "ast.h"
#include "parser.tab.h"
#include "svar.h"

extern void yyerror(const char *err);
extern char *get_token_string(int token);


t_ast_element *saffire_strCon(char *value) {
    t_ast_element *p;

    if ((p = malloc(sizeof(t_ast_element))) == NULL) {
        yyerror("Out of memory");
    }

    p->type = typeStrCon;
    p->strCon.value = strdup(value);

    return p;
}

t_ast_element *saffire_intCon(int value) {
    t_ast_element *p;

    if ((p = malloc(sizeof(t_ast_element))) == NULL) {
        yyerror("Out of memory");
    }

    p->type = typeIntCon;
    p->intCon.value = value;

    return p;
}

t_ast_element *saffire_var(char *var_name) {
    t_ast_element *p;

    if ((p = malloc(sizeof(t_ast_element))) == NULL) {
        yyerror("Out of memory");
    }

    p->type = typeVar;
    p->var.name = strdup(var_name);

    return p;
}

t_ast_element *saffire_opr(int opr, int nops, ...) {
    va_list ap;
    t_ast_element *p;

    if ((p = malloc(sizeof(t_ast_element))) == NULL) {
        yyerror("Out of memory");
    }
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

void saffire_free_node(t_ast_element *p) {
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

void saffire_print_node(t_ast_element *p, int recurse, int level) {
    printf("(%d) *** P->type: ", level);
    switch (p->type) {
        case typeStrCon :
            printf("typeStrCon\n");
            printf("str: '%s'\n", p->strCon.value);
            break;
        case typeIntCon :
            printf("typeIntCon\n");
            printf("int: %d\n", p->intCon.value);
            break;
        case typeVar :
            printf("typeVar\n");
            printf("name: '%s'\n", p->var.name);
            break;
        case typeOpr :
            printf("typeOpr\n");
            printf("operator : %d\n", p->opr.oper);
            printf("operators: %d\n", p->opr.nops);

            for (int i=0; i!=p->opr.nops; i++) {
                saffire_print_node(p->opr.ops[i], recurse, ++level);
            }
            break;
        default :
            printf("unknown (%d)\n", p->type);
            break;
    }
    printf("\n");
}

static int node_nr = 0;

void saffire_dot_node_iterate(FILE *f, t_ast_element *p, int link_node_nr) {
    int cur_node_nr = node_nr;
    node_nr++;
    fprintf(f, "\tN_%d [", cur_node_nr);
    switch (p->type) {
        case typeStrCon :
            fprintf(f, "style=rounded,label=\"{N:%d|Type=String|Value=\\\"%s\\\"}\"]\n", cur_node_nr, p->strCon.value);
            break;
        case typeIntCon :
            fprintf(f, "style=rounded,label=\"{N:%d|Type=Numerical|Value=%d}\"]\n", cur_node_nr, p->intCon.value);
            printf("int: %d\n", p->intCon.value);
            break;
        case typeVar :
            fprintf(f, "style=rounded,label=\"{N:%d|Type=Variable|Value=\\\"%s\\\"}\"]\n", cur_node_nr, p->var.name);
            printf("name: '%s'\n", p->var.name);
            break;
        case typeOpr :
            fprintf(f, "label=\"{N:%d|Type=Opr|Operator=%s (%d)| NrOps=%d} \"]\n", cur_node_nr, get_token_string(p->opr.oper), p->opr.oper, p->opr.nops);

            for (int i=0; i!=p->opr.nops; i++) {
                saffire_dot_node_iterate(f, p->opr.ops[i], cur_node_nr);
            }
            break;
        default :
            fprintf(f, "style=rounded,color=firebrick1,label=\"{N:%d|Type=UNKNOWN|Value=%d}\"]\n", node_nr, p->type);
            break;
    }

    if (link_node_nr != -1) {
        fprintf(f, "\tN_%d -> N_%d\n", link_node_nr, cur_node_nr);
    }
}

void saffire_dot_node(t_ast_element *p) {
    FILE *f = fopen("ast.dot", "w");
    if (!f) {
        printf("Cannot open ast.dot for writing\n");
        return;
    }

    fprintf(f, "# Generated by saffire_dot_node(). Generate with: dot -T png -o ast.png ast.dot\n");
    fprintf(f, "digraph G {\n");
    fprintf(f, "\tnode [ shape = record ];\n");
    fprintf(f, "\n");

    saffire_dot_node_iterate(f, p, -1);

    fprintf(f, "}\n");
    fclose(f);
}


/**
 *
 */
void saffire_execute(t_ast_element *p) {
    saffire_dot_node(p);
    saffire_print_node(p, 1, 0);
    saffire_compiler(p);
}

