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
#ifndef __COMPILER_SAFFIRE_PARSER_H__
#define __COMPILER_SAFFIRE_PARSER_H__

    #include "compiler/parser_helpers.h"
    #include "compiler/ast_nodes.h"
    #include "vm/frame.h"

// @TODO: Add VM running mode into this structure


    // @TODO: Should these be here?
    #define SAFFIRE_EXECMODE_REPL       1
    #define SAFFIRE_EXECMODE_FILE       2



    struct _pi_switch {
        int has_default;                // 1: a default is present. 0: no default
        struct _pi_switch *parent;      // Pointer to parent switch statement (or NULL when we are at the first)
    };

    /**
     * Global compile table with information needed during compilation of the code
     */
    struct _parserinfo {
        t_hash_table *constants;             // Table of all constants defined (globally)
        t_hash_table *classes;               // Table of all FQCN classes

        t_class *active_class;               // Current active class

        int in_class;                        // 1 when we are inside a class, 0 otherwise
        int in_loop_counter;                 // incremental loop counter. (deals with while() inside while() etc)
        int in_method;                       // 1 when we are inside a method, 0 otherwise
        struct _pi_switch *switches;        // Linked list of switch statements
        struct _pi_switch *current_switch;  // Pointer to the current switch statement (or NULL when not in switch)
    };

    struct SaffireParser {
        int     mode;                       // SAFFIRE_EXECMODE_* constants

        FILE    *file;                      // FILE to read from (or NULL)
        char    *filename;                  // Actual filename

        int     (*yyparse)(void *, int, char *, int);    // actual yyparse function, or NULL to use the default one
        void    *yyparse_args;              // Pointer to argument structure. Actual context known to the called yyparse method

        void    (*yyexec)(SaffireParser *); // Actual executor for the saffire parser ast.

        t_vm_frame *initial_frame;          // Initial frame to run the bytecode in

        t_parserinfo *parserinfo;           // Structure with all relevant parser info

        t_ast_element  *ast;                // Generated AST element.

        char    *error;                     // Returned error
        int     eof;                        // End of file reached, or user quits repl
    };

#endif
