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
#ifndef __COMPILER_PARSER_HELPERS_H__
#define __COMPILER_PARSER_HELPERS_H__

    #include "compiler/ast_nodes.h"
    #include "compiler/class.h"
    #include "general/hashtable.h"

//    #include "compiler/saffire_parser.h"
//    #include "compiler/parser.tab.h"
//    #include "compiler/lex.yy.h"

    /* @TODO: These should not be here */
    #define MODIFIER_PUBLIC              1
    #define MODIFIER_PROTECTED           2
    #define MODIFIER_PRIVATE             4
    #define MODIFIER_FINAL               8
    #define MODIFIER_ABSTRACT           16
    #define MODIFIER_STATIC             32

    #define MODIFIER_MASK_VISIBILITY      (MODIFIER_PUBLIC | MODIFIER_PROTECTED | MODIFIER_PRIVATE)

    typedef struct SaffireParser SaffireParser;
    typedef struct _parserinfo t_parserinfo;

    void parser_error(SaffireParser *sp, int lineno, const char *format, ...);

    t_parserinfo *alloc_parserinfo(void);
    void free_parserinfo(t_parserinfo *pi);

    void parser_init_class(SaffireParser *sp, int lineno, int modifiers, char *name, t_ast_element *extends, t_ast_element *implements);
    void parser_fini_class(SaffireParser *sp, int lineno);

    void parser_switch_case(SaffireParser *sp, int lineno);
    void parser_switch_default(SaffireParser *sp, int lineno);
    void parser_switch_end(SaffireParser *sp, int lineno);
    void parser_switch_begin(SaffireParser *sp, int lineno);

    void parser_check_label(SaffireParser *sp, int lineno, const char *name);

    void parser_check_permitted_identifiers(SaffireParser *sp, int lineno, const char *name);
    char *parser_build_var(SaffireParser *sp, int lineno, int argc, ...);

    void parser_validate_return(SaffireParser *sp, int lineno);
    void parser_validate_break(SaffireParser *sp, int lineno);
    void parser_validate_continue(SaffireParser *sp, int lineno);
    void parser_validate_breakelse(SaffireParser *sp, int lineno);

    void parser_loop_enter(SaffireParser *sp, int lineno);
    void parser_loop_leave(SaffireParser *sp, int lineno);

    void parser_init_method(SaffireParser *sp, int lineno, const char *name);
    void parser_fini_method(SaffireParser *sp, int lineno);
    void parser_validate_constant(SaffireParser *sp, int lineno, char *constant);
    void parser_validate_abstract_method_body(SaffireParser *sp, int lineno, long modifiers, t_ast_element *body);
    void parser_validate_class_modifiers(SaffireParser *sp, int lineno, long modifiers);
    void parser_validate_method_modifiers(SaffireParser *sp, int lineno, long modifiers);
    void parser_validate_property_modifiers(SaffireParser *sp, int lineno, long modifiers);
    void parser_validate_flags(SaffireParser *sp, int lineno, long cur_flags, long new_flag);

    char parser_mod_to_visibility(SaffireParser *sp, int lineno, long modifiers);
    char parser_mod_to_methodflags(SaffireParser *sp, int lineno, long modifiers);

#endif
