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
#ifndef __COMPILER_PARSER_H__
#define __COMPILER_PARSER_H__

    #include "compiler/ast_nodes.h"
    #include "compiler/class.h"
    #include "general/hashtable.h"

    #include "compiler/saffire_parser.h"
    #include "compiler/parser.tab.h"
    #include "compiler/lex.yy.h"

    /* @TODO: These should not be here */
    #define MODIFIER_PUBLIC              1
    #define MODIFIER_PROTECTED           2
    #define MODIFIER_PRIVATE             4
    #define MODIFIER_FINAL               8
    #define MODIFIER_ABSTRACT           16
    #define MODIFIER_STATIC             32

    #define MODIFIER_MASK_VISIBILITY      (MODIFIER_PUBLIC | MODIFIER_PROTECTED | MODIFIER_PRIVATE)


    typedef struct switch_struct {
        int has_default;                // 1: a default is present. 0: no default
        struct switch_struct *parent;   // Pointer to parent switch statement (or NULL when we are at the first)
    } t_switch_struct;

    /**
     * Global compile table with information needed during compilation of the code
     */
    typedef struct global_table {
        t_hash_table *constants;          // Table of all constants defined (globally)
        t_hash_table *classes;            // Table of all FQCN classes

        t_class *active_class;            // Current active class

        int in_class;                     // 1 when we are inside a class, 0 otherwise
        int in_loop_counter;              // incremental loop counter. (deals with while() inside while() etc)
        int in_method;                    // 1 when we are inside a method, 0 otherwise
        t_switch_struct *switches;        // Linked list of switch statements
        t_switch_struct *current_switch;  // Pointer to the current switch statement (or NULL when not in switch)
    } t_global_table;

    t_global_table *global_table;      // A global table with compilation info

    void parser_init(void);
    void parser_fini(void);

    void parser_init_class(int lineno, int modifiers, char *name, t_ast_element *extends, t_ast_element *implements);
    void parser_fini_class(int lineno);

    void parser_switch_case(int lineno);
    void parser_switch_default(int lineno);
    void parser_switch_end(int lineno);
    void parser_switch_begin(int lineno);

    void parser_check_label(int lineno, const char *name);

    void parser_check_permitted_identifiers(int lineno, const char *name);
    char *parser_build_var(int argc, ...);

    void parser_validate_return(int lineno);
    void parser_validate_break(int lineno);
    void parser_validate_continue(int lineno);
    void parser_validate_breakelse(int lineno);

    void parser_loop_enter(void);
    void parser_loop_leave(int lineno);

    void parser_init_method(const char *name);
    void parser_fini_method(void);
    void parser_validate_constant(int lineno, char *constant);
    void parser_validate_abstract_method_body(int lineno, long modifiers, t_ast_element *body);
    void parser_validate_class_modifiers(int lineno, long modifiers);
    void parser_validate_method_modifiers(int lineno, long modifiers);
    void parser_validate_property_modifiers(int lineno, long modifiers);
    void parser_validate_flags(int lineno, long cur_flags, long new_flag);

    char parser_mod_to_visibility(long modifiers);
    char parser_mod_to_methodflags(long modifiers);

#endif
