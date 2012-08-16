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
#ifndef __AST_H__
#define __AST_H__

    #include "svar.h"

    // different kind of nodes we manage
    typedef enum { typeStrCon, typeIntCon, nullVar, typeVar, typeOpr } nodeEnum;

    typedef struct {
        char *value;                // Pointer to the actual constant string
    } strConNode;

    typedef struct {
        int value;                  // Integer constant
    } intConNode;

    typedef struct {
        char *name;                 // Name of the actual variable to use
    } varNode;

    typedef struct {
        int oper;                   // Operator
        int nops;                   // number of additional operands
        struct ast_element **ops;   // Operands (should be max of 2: left and right)
    } oprNode;


    typedef struct ast_element {
        nodeEnum type;              // Type of the node
        union {
            intConNode intCon;        // constant int
            strConNode strCon;        // constant string
            varNode var;              // variable
            oprNode opr;              // operator
        };
    } t_ast_element;


    // actual root element
    t_ast_element *ast_root;

    t_ast_element *ast_strCon(char *value);
    t_ast_element *ast_intCon(int value);
    t_ast_element *ast_var(char *var_name);
    t_ast_element *ast_opr(int opr, int nops, ...);
    t_ast_element *ast_add(t_ast_element *src, t_ast_element *new_element);
    t_ast_element *ast_nop(void);

    void ast_free_node(t_ast_element *p);

#endif