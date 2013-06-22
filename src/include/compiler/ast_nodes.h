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
#ifndef __AST_NODES_H__
#define __AST_NODES_H__

    #define ID_LOAD     0
    #define ID_STORE    1

    #include "compiler/class.h"
    #include "general/dll.h"
    #include <stdio.h>

    // different kind of nodes we manage
    typedef enum { typeAstString, typeAstNumerical, typeAstNull, typeAstIdentifier,
                   typeAstOpr, typeAstNop, typeAstGroup,
                   typeAstClass, typeAstInterface, typeAstAttribute, typeAstProperty,
                   typeAstAssignment, typeAstComparison, typeAstBool, typeAstOperator,
                 } nodeEnum;

    typedef struct {
        char *value;                // Pointer to the actual constant string
    } stringNode;

    typedef struct {
        int value;                  // Integer constant
    } numericalNode;

    typedef struct {
        char *name;                 // Name of the actual variable to use
    } identifierNode;

    typedef struct {
        struct _ast_element *class;
        struct _ast_element *property;
    } propertyNode;

    typedef struct {
        char *name;                         // Name of the attribute
        char attrib_type;                   // Type (constant, property, method)
        char access;                        // RO / RW
        char visibility;                    // Public, private, protected)
        struct _ast_element *value;         // Actual value

        // Additional information for method attributes
        char method_flags;                  // Flags: static, final, abstract
        struct _ast_element *arguments;     // Additional arguments
    } attributeNode;

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
    } operatorNode;

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
        int len;
        struct _ast_element **items;
    } groupNode;

    typedef struct _ast_element {
        nodeEnum type;                  // Type of the node
        int flags;                      // Current flag (used for interpreting)
        unsigned long lineno;           // Current line number for this AST element
        union {
            numericalNode numerical;    // constant int
            stringNode string;          // constant string
            identifierNode identifier;  // variable
            propertyNode property;      // property
            attributeNode attribute;    // attribute
            oprNode opr;                // operators
            groupNode group;            // grouping of multiple items (statements etc)
            classNode class;            // class
            interfaceNode interface;    // interface
            assignmentNode assignment;  // assignment
            comparisonNode comparison;  // comparison
            boolopNode boolop;          // booleans (|| &&)
            operatorNode operator;      // operators (+ - * / etc)
        };
    } t_ast_element;

    t_ast_element *ast_node_string(int lineno, char *value);
    t_ast_element *ast_node_string_dup(int lineno, t_ast_element *src);
    t_ast_element *ast_node_numerical(int lineno, int value);
    t_ast_element *ast_node_identifier(int lineno, char *var_name);
    t_ast_element *ast_node_property(int lineno, t_ast_element *class, t_ast_element *property);
    t_ast_element *ast_node_opr(int lineno, int opr, int nops, ...);
    t_ast_element *ast_node_group(int len, ...);
    t_ast_element *ast_node_add(t_ast_element *src, t_ast_element *new_element);
    t_ast_element *ast_node_string_concat(t_ast_element *src, char *s);
    t_ast_element *ast_node_concat(t_ast_element *src, char *s);
    t_ast_element *ast_node_class(int lineno, t_class *class, t_ast_element *body);
    t_ast_element *ast_node_interface(int lineno, int modifiers, char *name, t_ast_element *implements, t_ast_element *body);
    t_ast_element *ast_node_nop(void);
    t_ast_element *ast_node_null(void);
    t_ast_element *ast_node_assignment(int lineno, int op, t_ast_element *left, t_ast_element *right);
    t_ast_element *ast_node_comparison(int lineno, int cmp, t_ast_element *left, t_ast_element *right);
    t_ast_element *ast_node_operator(int lineno, int op, t_ast_element *left, t_ast_element *right);
    t_ast_element *ast_node_boolop(int lineno, int boolop, t_ast_element *left, t_ast_element *right);
    t_ast_element *ast_node_attribute(int lineno, char *name, char attrib_type, char visibility, char access, t_ast_element *value, char method_flags, t_ast_element *arguments);


    t_ast_element *ast_generate_from_file(const char *filename);
    t_ast_element *ast_generate_tree(FILE *fp, char *filename, int mode);
    void ast_free_node(t_ast_element *p);

#endif
