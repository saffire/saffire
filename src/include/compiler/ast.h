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
#ifndef __AST_H__
#define __AST_H__

    #define ID_LOAD     0
    #define ID_STORE    1

    #include "compiler/class.h"
    #include "general/dll.h"
    #include <stdio.h>

    // different kind of nodes we manage
    typedef enum { typeAstString, typeAstNumerical, typeAstNull, typeAstIdentifier,
                   typeAstOpr,
                   typeAstClass, typeAstInterface, typeAstMethod,
                   typeAstAssignment, typeAstComparison, typeAstBool
                 } nodeEnum;

    typedef struct {
        char *value;                // Pointer to the actual constant string
    } stringNode;

    typedef struct {
        int value;                  // Integer constant
    } numericalNode;

    typedef struct {
        char *name;                 // Name of the actual variable to use
        char action;                // 0 = LOAD, 1 = STORE
    } identifierNode;

    typedef struct {
        int oper;                   // Operator
        int nops;                   // number of additional operands
        struct _ast_element **ops;   // Operands (should be max of 2: left and right)
    } oprNode;

    typedef struct {
        int op;
        struct _ast_element *l;
        struct _ast_element *r;
    } assignmentNode;

    typedef struct {
        int cmp;
        struct _ast_element *l;
        struct _ast_element *r;
    } comparisonNode;

    typedef struct {
        int op;
        struct _ast_element *l;
        struct _ast_element *r;
    } boolopNode;

    typedef struct {
        int modifiers;
        char *name;
        struct _ast_element *extends;
        struct _ast_element *implements;
        struct _ast_element *body;
    } classNode;

    typedef struct {
        int modifiers;
        char *name;
        struct _ast_element *implements;
        struct _ast_element *body;
    } interfaceNode;

    typedef struct {
        int modifiers;
        char *name;
        struct _ast_element *arguments;
        struct _ast_element *body;
    } methodNode;

    typedef void (*t_clean_handler)(t_dll*);

    typedef struct _ast_element {
        nodeEnum type;              // Type of the node
        int flags;                  // Current flag (used for interpreting)
        unsigned long lineno;                 // Current line number for this AST element
        t_clean_handler clean_handler;       //Function to clean up
        union {
            numericalNode numerical;    // constant int
            stringNode string;          // constant string
            identifierNode identifier;  // variable
            oprNode opr;                // operators
            classNode class;            // class
            interfaceNode interface;    // interface
            methodNode method;          // methods
            assignmentNode assignment;  // assignment
            comparisonNode comparison;  // comparison
            boolopNode boolop;          // booleans (|| &&)
        };
    } t_ast_element;


    t_ast_element *ast_generate_from_file(const char *);
    t_ast_element *ast_generate_tree(FILE *fp);

    t_ast_element *ast_string(char *value);
    t_ast_element *ast_string_dup(t_ast_element *src);
    t_ast_element *ast_numerical(int value);
    t_ast_element *ast_identifier(char *var_name, char action);
    t_ast_element *ast_opr(int opr, int nops, ...);
    t_ast_element *ast_add(t_ast_element *src, t_ast_element *new_element);
    t_ast_element *ast_add_children(t_ast_element *src, t_ast_element *new_element);
    t_ast_element *ast_string_concat(t_ast_element *src, char *s);
    t_ast_element *ast_concat(t_ast_element *src, char *s);
    t_ast_element *ast_class(t_class *class, t_ast_element *body);
    t_ast_element *ast_interface(int modifiers, char *name, t_ast_element *implements, t_ast_element *body);
    t_ast_element *ast_method(int modifiers, char *name, t_ast_element *arguments, t_ast_element *body);
    t_ast_element *ast_nop(void);
    t_ast_element *ast_assignment(int op, t_ast_element *left, t_ast_element *right);
    t_ast_element *ast_comparison(int cmp, t_ast_element *left, t_ast_element *right);
    t_ast_element *ast_boolop(int boolop, t_ast_element *left, t_ast_element *right);


    void ast_free_node(t_ast_element *p);

#endif