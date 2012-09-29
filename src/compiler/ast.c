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
#include "compiler/saffire_compiler.h"
#include "compiler/parser.tab.h"
#include "general/svar.h"
#include "general/smm.h"
#include "compiler/ast.h"

//extern void yylex_destroy
extern int yylex_destroy();
extern void yyerror(const char *err);
extern int yyparse();
extern FILE *yyin;


/**
 * Compile a a file into an AST (through bison). Returns the AST root node.
 */
t_ast_element *ast_compile_tree(FILE *fp) {
    // Initialize system
    svar_init_table();
    sfc_init();

    // Parse the file input, will return the tree in the global ast_root variable
    yyin = fp;
    yyparse();
    yylex_destroy();

    sfc_fini();
    svar_fini_table();

    // Returning a global var. We should change this by having the root node returned by yyparse() if this is possible
    return ast_root;
}


/**
 * Allocate a new element
 */
static t_ast_element *ast_alloc_element(void) {
    t_ast_element *p;

    if ((p = smm_malloc(sizeof(t_ast_element))) == NULL) {
        yyerror("Out of memory");   /* LCOV_EXCL_LINE */
    }

    return p;
}


/**
 * Creates a string node
 */
t_ast_element *ast_string(char *value) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeString;
    p->string.value = smm_strdup(value);

    return p;
}


/**
 * Creates a numerical node
 */
t_ast_element *ast_numerical(int value) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeNumerical;
    p->numerical.value = value;

    return p;
}


/**
 * Creates a identifier node
 */
t_ast_element *ast_identifier(char *var_name) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeIdentifier;
    p->identifier.name = smm_strdup(var_name);

    return p;
}


/**
 * Creates a null/nop node
 */
t_ast_element *ast_nop(void) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeNull;

    return p;
}


/**
 * Add a node to an existing operator node. This allows to have multiple children in later stages (like lists)
 */
t_ast_element *ast_add(t_ast_element *src, t_ast_element *new_element) {
    if (src->type != typeOpr) {
        yyerror("Cannot add to non-opr element");   /* LCOV_EXCL_LINE */
    }

    // Resize operator memory
    src->opr.ops = smm_realloc(src->opr.ops, src->opr.nops * sizeof(t_ast_element));
    if (src->opr.ops == NULL) {
        yyerror("Out of memory");   /* LCOV_EXCL_LINE */
    }

    // Add new operator
    src->opr.ops[src->opr.nops] = new_element;
    src->opr.nops++;

    return src;
}


/**
 * Create an operator node
 */
t_ast_element *ast_opr(int opr, int nops, ...) {
    t_ast_element *p = ast_alloc_element();
    va_list ap;

    if (nops && (p->opr.ops = smm_malloc (nops * sizeof(t_ast_element))) == NULL) {
        yyerror("Out of memory");   /* LCOV_EXCL_LINE */
    }

    p->type = typeOpr;
    p->opr.oper = opr;
    p->opr.nops = nops;

    // Add additional nodes (they can be added later with ast_add())
    if (nops) {
        va_start(ap, nops);
        for (int i=0; i < nops; i++) {
            p->opr.ops[i] = va_arg(ap, t_ast_element *);
        }
        va_end(ap);
    }

    return p;
}


/**
 * Create a class node.
 */
t_ast_element *ast_class(t_class *class, t_ast_element *body) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeClass;
    p->class.modifiers = class->modifiers;
    p->class.name = smm_strdup(class->name);

    p->class.extends = class->extends;
    p->class.implements = class->implements;

    p->class.body = body;

    return p;
}


/**
 * Create an interface node
 */
t_ast_element *ast_interface(int modifiers, char *name, t_ast_element *implements, t_ast_element *body) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeInterface;
    p->interface.modifiers = modifiers;
    p->interface.name = smm_strdup(name);
    p->interface.implements = implements;
    p->interface.body = body;

    return p;
}


/**
 * Create a method node
 */
t_ast_element *ast_method(int modifiers, char *name, t_ast_element *arguments, t_ast_element *body) {
    t_ast_element *p = ast_alloc_element();

    p->type = typeMethod;
    p->method.modifiers = modifiers;
    p->method.name = smm_strdup(name);
    p->method.arguments = arguments;
    p->method.body = body;

    return p;
}


/**
 * Free up an AST node
 */
void ast_free_node(t_ast_element *p) {
    if (!p) return;

    switch (p->type) {
        case typeString :
            smm_free(p->string.value);
            break;
        case typeIdentifier :
            smm_free(p->identifier.name);
            break;
        case typeClass :
            smm_free(p->class.name);
            ast_free_node(p->class.extends);
            ast_free_node(p->class.implements);
            ast_free_node(p->class.body);
            break;
        case typeInterface :
            smm_free(p->interface.name);
            ast_free_node(p->interface.implements);
            ast_free_node(p->interface.body);
            break;
        case typeMethod :
            smm_free(p->method.name);
            ast_free_node(p->method.arguments);
            ast_free_node(p->method.body);
            break;
        case typeOpr :
            if (p->opr.nops) {
                for (int i=0; i < p->opr.nops; i++) {
                    ast_free_node(p->opr.ops[i]);
                }
                smm_free(p->opr.ops);
            }
            break;
    }
    smm_free(p);
}
