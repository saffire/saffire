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
%{


#define YY_HEADER_EXPORT_START_CONDITIONS 1

    #include <stdio.h>
    #include "general/output.h"
    #include "general/smm.h"
    #include "compiler/lex.yy.h"
    #include "compiler/compiler.h"
    #include "compiler/ast.h"
    #include "debug.h"

    extern int yylineno;
    int yylex(void);
    void yyerror(unsigned long *ast, const char *err) { error_and_die(1, "line %lu: %s\n", (unsigned long)yylineno, err); exit(1); }

    // Use our own saffire memory manager
    void *yyalloc (size_t bytes) {
        return smm_malloc(bytes);
    }
    void *yyrealloc (void *ptr, size_t bytes) {
        return smm_realloc(ptr, bytes);
    }
    void yyfree (void *ptr) {
        return smm_free(ptr);
    }

    void saffire_push_state(int state);
    void saffire_pop_state();

    #ifdef __DEBUG
        #define YYDEBUG 1
        #define TRACE
    #else
        //#define YYDEBUG 0
        #define TRACE
    #endif

    #define YYPRINT(file, type, value)
%}

%union {
    char                 *sVal;
    long                 lVal;
    double               dVal;
    struct _ast_element  *nPtr;
}

%token END 0 "end of file"
%token <lVal> T_LNUM
%token <sVal> T_STRING T_IDENTIFIER T_REGEX

%type <lVal> modifier modifier_list assignment_operator comparison_operator

%token T_WHILE T_IF T_USE T_AS T_DO T_SWITCH T_FOR T_FOREACH T_CASE
%nonassoc T_ELSE

%token T_OP_INC T_OP_DEC T_PLUS_ASSIGNMENT T_MINUS_ASSIGNMENT T_MUL_ASSIGNMENT T_DIV_ASSIGNMENT
%token T_MOD_ASSIGNMENT T_AND_ASSIGNMENT T_OR_ASSIGNMENT T_XOR_ASSIGNMENT T_SL_ASSIGNMENT T_SR_ASSIGNMENT

%token T_CATCH T_BREAK T_GOTO T_BREAKELSE T_CONTINUE T_THROW T_RETURN T_FINALLY T_TRY T_DEFAULT T_METHOD
%token T_SELF T_PARENT T_NS_SEP
%left T_ASSIGNMENT T_GE T_LE T_EQ T_NE '>' '<' '^' T_IN T_RE T_REGEX
%left '+' '-'
%left '*' '/'
%token T_AND T_OR T_SHIFT_LEFT T_SHIFT_RIGHT
%token T_CLASS T_EXTENDS T_ABSTRACT T_FINAL T_IMPLEMENTS T_INTERFACE
%token T_PUBLIC T_PRIVATE T_PROTECTED T_CONST T_STATIC T_READONLY T_PROPERTY
%token T_LABEL T_METHOD_CALL T_ARITHMIC T_LOGICAL T_TOP_STATEMENTS T_PROGRAM T_USE_STATEMENTS
%token T_FQN T_ARGUMENT_LIST T_LIST T_STATEMENTS T_ASSIGNMENT T_FIELDACCESS
%token T_MODIFIERS T_CONSTANTS T_DATA_ELEMENTS T_DATA_STRUCTURE T_DATA_ELEMENT T_METHOD_ARGUMENT
%token T_IMPORT T_FROM T_ELLIPSIS

/* reserved for later use */
%token T_YIELD

%type <nPtr> program use_statement_list non_empty_use_statement_list use_statement top_statement_list
%type <nPtr> non_empty_top_statement_list top_statement class_definition interface_definition
%type <nPtr> constant_list statement_list compound_statement statement expression_statement jump_statement
%type <nPtr> label_statement selection_statement iteration_statement class_list while_statement
%type <nPtr> guarding_statement expression catch_list catch data_structure_element data_structure_elements
%type <nPtr> class_interface_implements method_argument_list interface_or_abstract_method_definition class_extends
%type <nPtr> non_empty_method_argument_list interface_inner_statement_list class_inner_statement class_inner_statement_list
%type <nPtr> method_call real_postfix_expression data_structure_name
%type <nPtr> postfix_expression unary_expression primary_expression arithmic_unary_operator
%type <nPtr> logical_unary_operator multiplicative_expression additive_expression shift_expression regex_expression
%type <nPtr> catch_header conditional_expression assignment_expression real_scalar_value
%type <nPtr> constant method_argument interface_inner_statement interface_method_definition interface_property_definition
%type <nPtr> class_method_definition class_property_definition qualified_name calling_method_argument_list
%type <nPtr> data_structure logical_unary_expression equality_expression and_expression inclusive_or_expression
%type <nPtr> conditional_or_expression exclusive_or_expression conditional_and_expression case_statements case_statement
%type <nPtr> special_name method_call_pre_parenthesis namespace_identifier scalar_value

%type <sVal> T_ASSIGNMENT T_PLUS_ASSIGNMENT T_MINUS_ASSIGNMENT T_MUL_ASSIGNMENT T_DIV_ASSIGNMENT T_MOD_ASSIGNMENT T_AND_ASSIGNMENT
%type <sVal> T_OR_ASSIGNMENT T_XOR_ASSIGNMENT T_SL_ASSIGNMENT T_SR_ASSIGNMENT '~' '!' '+' '-' T_SELF T_PARENT

%token_table
%error-verbose

%start saffire


%parse-param { unsigned long *ast_root }

%% /* rules */


/**
 ************************************************************
 *                  TOP AND USE STATEMENTS
 ************************************************************
 */

saffire:
        program { TRACE *ast_root = (unsigned long)$1; }
;

program:
        use_statement_list top_statement_list { TRACE $$ = ast_opr(T_PROGRAM,2, $1, $2); }
;

use_statement_list:
        non_empty_use_statement_list { TRACE $$ = $1; }
    |   /* empty */                  { TRACE $$ = ast_nop(); }
;

non_empty_use_statement_list:
        use_statement { TRACE $$ = ast_opr(T_USE_STATEMENTS, 1, $1); }
    |   non_empty_use_statement_list use_statement { TRACE $$ = ast_add($$, $2); }
;

use_statement:
        /* use <foo> as <bar>; */
        T_USE namespace_identifier T_AS T_IDENTIFIER ';' { TRACE $$ = ast_opr(T_USE, 2, $2, ast_string($4));  }
        /* use <foo>; */
    |   T_USE namespace_identifier                   ';' { TRACE $$ = ast_opr(T_USE, 1, $2); }
        /* import <foo> from <bar> */
    |   T_IMPORT namespace_identifier                   T_FROM namespace_identifier ';' { TRACE $$ = ast_opr(T_IMPORT, 3, $2, ast_nop(), $4); }
        /* import <foo> as <bar> from <baz> */
    |   T_IMPORT namespace_identifier T_AS T_IDENTIFIER T_FROM namespace_identifier ';' { TRACE $$ = ast_opr(T_IMPORT, 3, $2, ast_string($4), $6); }
        /* import <foo> as <bar> */
    |   T_IMPORT namespace_identifier T_AS T_IDENTIFIER                             ';' { TRACE $$ = ast_opr(T_IMPORT, 3, $2, ast_string($4), ast_string_dup($2)); }
        /* import <foo> */
    |   T_IMPORT namespace_identifier                                               ';' { TRACE $$ = ast_opr(T_IMPORT, 3, $2, ast_string_dup($2), ast_string_dup($2)); }
;


namespace_identifier:
       T_IDENTIFIER                                 { TRACE $$ = ast_string($1); }
    |  T_NS_SEP T_IDENTIFIER                        { TRACE $$ = ast_string("::"); $$ = ast_string_concat($$, $2); }
    |  namespace_identifier T_NS_SEP T_IDENTIFIER   { TRACE $$ = ast_string_concat($$, "::"); $$ = ast_string_concat($$, $3); }
;



/* Top statements are single (global) statements and/or class/interface/constant */
top_statement_list:
        non_empty_top_statement_list { TRACE $$ = $1; }
    |   /* empty */                  { TRACE $$ = ast_nop(); }
;

non_empty_top_statement_list:
        top_statement                              { TRACE $$ = ast_opr(T_TOP_STATEMENTS, 1, $1); }
    |   non_empty_top_statement_list top_statement { TRACE $$ = ast_add($$, $2); }
;

/* Top statements can be classes, interfaces, constants, statements */
top_statement:
        class_definition        { TRACE $$ = $1; }
    |   interface_definition    { TRACE $$ = $1; }
    |   constant_list           { TRACE $$ = $1; }
    |   statement_list          { TRACE $$ = $1; }
;


/**
 ************************************************************
 *                 BLOCKS & STATEMENTS
 ************************************************************
 */

statement_list:
        statement                { TRACE $$ = ast_opr(T_STATEMENTS, 1, $1); }
    |   statement_list statement { TRACE $$ = ast_add($$, $2); }
;

statement:
        label_statement        { TRACE $$ = $1; }
    |   compound_statement     { TRACE $$ = $1; }
    |   jump_statement         { TRACE $$ = $1; }
    |   expression_statement   { TRACE $$ = $1; }
    |   selection_statement    { TRACE $$ = $1; }
    |   iteration_statement    { TRACE $$ = $1; }
    |   guarding_statement     {  TRACE $$ = $1; }
;

/* A compound statement is a (set of) statement captured by curly brackets */
compound_statement:
        '{' '}'                 { TRACE $$ = ast_nop(); }
    |   '{' statement_list '}'  { TRACE $$ = $2; }
;

/* if if/else or switch() statements */
selection_statement:
        T_IF conditional_expression statement                  { TRACE $$ = ast_opr(T_IF, 2, $2, $3); }
    |   T_IF conditional_expression statement T_ELSE statement { TRACE $$ = ast_opr(T_IF, 3, $2, $3, $5); }
    |   T_SWITCH '(' conditional_expression ')' { sfc_loop_enter(); sfc_switch_begin(); } '{' case_statements '}' { sfc_loop_leave(); sfc_switch_end(); TRACE $$ = ast_opr(T_SWITCH, 2, $3, $7); }
;

case_statements:
        case_statement { TRACE $$ = ast_opr(T_STATEMENTS, 1, $1); }
    |   case_statements case_statement { TRACE $$ = ast_add($$, $2); }
;

case_statement:
        T_CASE conditional_expression ':'   { sfc_switch_case(); }    statement_list { TRACE $$ = ast_opr(T_CASE, 2, $2, $5); }
    |   T_CASE conditional_expression ':'   { sfc_switch_case(); }    { TRACE $$ = ast_opr(T_CASE, 2, $2, ast_nop()); }
    |   T_DEFAULT ':'           { sfc_switch_default(); } statement_list { TRACE $$ = ast_opr(T_DEFAULT, 1, $4); }
    |   T_DEFAULT ':'           { sfc_switch_default(); } { TRACE $$ = ast_opr(T_DEFAULT, 1, ast_nop()); }
;


/* while, while else, do/while, for and foreach */
iteration_statement:
        while_statement T_ELSE statement { TRACE $$ = ast_add($1, $3); }
    |   while_statement { TRACE $$ = $1; }

    |   T_DO { sfc_loop_enter(); } statement T_WHILE '(' conditional_expression ')' ';' { sfc_loop_leave(); TRACE $$ = ast_opr(T_DO, 2, $3, $6); }

    |   T_FOR '(' expression_statement expression_statement            ')' { sfc_loop_enter(); } statement { TRACE $$ = ast_opr(T_FOR, 3, $3, $4, $7); }
    |   T_FOR '(' expression_statement expression_statement expression ')' { sfc_loop_enter(); } statement { TRACE $$ = ast_opr(T_FOR, 4, $3, $4, $5, $8); }

    |   T_FOREACH '(' expression T_AS expression ')' { sfc_loop_enter(); } statement { sfc_loop_leave(); TRACE $$ = ast_opr(T_FOREACH, 3, $3, $5, $8); }
;

/* while is separate otherwise we cannot find it's else */
while_statement:
        T_WHILE '(' conditional_expression ')' { sfc_loop_enter(); } statement { sfc_loop_leave(); TRACE $$ = ast_opr(T_WHILE, 2, $3, $6); }
;

/* An expression is anything that evaluates something */
expression_statement:
        ';'             { TRACE $$ = ast_opr(';', 0); }
    |   expression ';'  { TRACE $$ = $1; DEBUG_PARSEPRINT("\n\nLine %d:\n", yylineno+1); }
;


/* Jumps to another part of the code */
jump_statement:
        T_BREAK ';'                 { saffire_validate_break(); TRACE $$ = ast_opr(T_BREAK, 0); }
    |   T_BREAKELSE ';'             { saffire_validate_breakelse(); TRACE $$ = ast_opr(T_BREAKELSE, 0); }
    |   T_CONTINUE ';'              { saffire_validate_continue(); TRACE $$ = ast_opr(T_CONTINUE, 0); }
    |   T_RETURN ';'                { saffire_validate_return(); TRACE $$ = ast_opr(T_RETURN, 0); }
    |   T_RETURN expression ';'     { saffire_validate_return(); TRACE $$ = ast_opr(T_RETURN, 1, $2); }
    |   T_THROW expression ';'      { TRACE $$ = ast_opr(T_THROW, 1, $2); }
    |   T_GOTO T_IDENTIFIER ';'     { TRACE $$ = ast_opr(T_GOTO, 1, ast_string($2)); smm_free($2); }
    |   T_GOTO T_LNUM ';'           { TRACE $$ = ast_opr(T_GOTO, 1, ast_numerical($2)); }
;

/* try/catch try/catch/finally blocks */
guarding_statement:
        T_TRY compound_statement catch_list                               { TRACE $$ = ast_opr(T_TRY, 2, $2, $3); }
    |   T_TRY compound_statement catch_list T_FINALLY compound_statement  { TRACE $$ = ast_opr(T_FINALLY, 3, $2, $3, $5); }
;

/* There can be multiple catches on a try{} block */
catch_list:
        catch            { TRACE $$ = $1; }
    |   catch_list catch { TRACE $$ = ast_opr(T_LIST, 2, $1, $2); }
;

catch:
        catch_header compound_statement { TRACE $$ = ast_opr(T_LIST, 2, $1, $2); }
;

catch_header:
        T_CATCH '(' T_IDENTIFIER T_IDENTIFIER ')' { TRACE $$ = ast_opr(T_CATCH, 2, ast_string($3), ast_string($4)); smm_free($3); smm_free($4); }
;

label_statement:
        T_IDENTIFIER ':'    { saffire_check_label($1); TRACE $$ = ast_opr(T_LABEL, 1, ast_string($1)); smm_free($1); }
    |   T_LNUM ':'          { TRACE $$ = ast_opr(T_LABEL, 1, ast_numerical($1)); }
;


/**
 ************************************************************
 *                  ASSIGNMENT & EXPRESSION
 ************************************************************
 */

/*
 * Order of precedence is generated by changing expressions instead of using %left %right keywords.
 */

expression:
        assignment_expression { TRACE $$ = $1; }
    |   expression ',' assignment_expression { TRACE $$ = ast_add($$, $3); }
    |   expression ',' ',' assignment_expression { TRACE $$ = ast_add($$, ast_nop()); $$ = ast_add($$, $4); }

;

assignment_expression:
        conditional_expression { TRACE $$ = $1; }
    |   unary_expression assignment_operator assignment_expression { TRACE $$ = ast_assignment($2, $1, $3); }
;

conditional_expression:
        conditional_or_expression { TRACE $$ = $1; }
    |   conditional_or_expression '?' expression ':' conditional_expression { TRACE $$ = ast_opr('?', 2, $1, $3); }
;

conditional_or_expression:
        conditional_and_expression { TRACE $$ = $1; }
    |   conditional_or_expression T_OR conditional_and_expression { TRACE $$ = ast_boolop(1, $1, $3);}
;

conditional_and_expression:
        inclusive_or_expression { TRACE $$ = $1; }
    |   conditional_and_expression T_AND inclusive_or_expression { TRACE $$ = ast_boolop(0, $1, $3); }
;

inclusive_or_expression:
        exclusive_or_expression { TRACE $$ = $1; }
    |   inclusive_or_expression '|' exclusive_or_expression { TRACE $$ = ast_opr('|', 2, $1, $3);}
;

exclusive_or_expression:
        and_expression { TRACE $$ = $1; }
    |   exclusive_or_expression '^' and_expression { TRACE $$ = ast_opr('^', 2, $1, $3); }
;

and_expression:
        equality_expression { TRACE $$ = $1; }
    |   and_expression '&' equality_expression { TRACE $$ = ast_opr('&', 2, $1, $3); }
;

equality_expression:
        regex_expression { TRACE $$ = $1; }
    |   equality_expression comparison_operator regex_expression { TRACE $$ = ast_comparison($2, $1, $3); }
;

comparison_operator:
        T_EQ { TRACE $$ = T_EQ; }
    |   T_NE { TRACE $$ = T_NE; }
    |   T_IN { TRACE $$ = T_IN; }
    |   '>'  { TRACE $$ = '>';  }
    |   '<'  { TRACE $$ = '<';  }
    |   T_LE { TRACE $$ = T_LE; }
    |   T_GE { TRACE $$ = T_GE; }
;

regex_expression:
        shift_expression { TRACE $$ = $1; }
    |   regex_expression T_RE T_REGEX { TRACE $$ = ast_opr(T_RE, 2, $1, ast_string($3)); smm_free($3); }
;

shift_expression:
        additive_expression { TRACE $$ = $1; }
    |   shift_expression T_SHIFT_LEFT additive_expression { TRACE $$ = ast_opr(T_SHIFT_LEFT, 2, $1, $3); }
    |   shift_expression T_SHIFT_RIGHT additive_expression { TRACE $$ = ast_opr(T_SHIFT_RIGHT, 2, $1, $3); }
;

additive_expression:
        multiplicative_expression { TRACE $$ = $1; }
    |   additive_expression '+' multiplicative_expression { TRACE $$ = ast_opr('+', 2, $1, $3); }
    |   additive_expression '-' multiplicative_expression { TRACE $$ = ast_opr('-', 2, $1, $3); }
;

multiplicative_expression:
        unary_expression { TRACE $$ = $1; }
    |   multiplicative_expression '*' unary_expression { TRACE $$ = ast_opr('*', 2, $1, $3); }
    |   multiplicative_expression '/' unary_expression { TRACE $$ = ast_opr('/', 2, $1, $3); }
    |   multiplicative_expression '%' unary_expression { TRACE $$ = ast_opr('%', 2, $1, $3); }
;

unary_expression:
        T_OP_INC unary_expression                   { TRACE $$ = ast_opr(T_OP_INC, 1, $2); }
    |   T_OP_DEC unary_expression                   { TRACE $$ = ast_opr(T_OP_DEC, 1, $2); }
    |   arithmic_unary_operator primary_expression  { TRACE $$ = ast_opr(T_ARITHMIC, 2, $1, $2); }
    |   logical_unary_expression                    { TRACE $$ = $1; }
;

postfix_expression:
        primary_expression      { TRACE $$ = $1; }
    |   real_postfix_expression { TRACE $$ = $1; }
;

real_postfix_expression:
        postfix_expression T_OP_INC { TRACE $$ = ast_opr(T_OP_INC, 1, $1); }
    |   postfix_expression T_OP_DEC { TRACE $$ = ast_opr(T_OP_DEC, 1, $1); }
;

logical_unary_expression:
        postfix_expression                      { TRACE $$ = $1; }
    |   logical_unary_operator unary_expression { TRACE $$ = ast_opr(T_LOGICAL, 2, $1, $2); }
;

primary_expression:
        qualified_name          { TRACE $$ = $1; }
    |   method_call             { TRACE $$ = $1; }
    |   special_name            { TRACE $$ = $1; }
    |   data_structure          { TRACE $$ = $1; }
    |   real_scalar_value       { TRACE $$ = $1; }
    |   '(' expression ')'      { TRACE $$ = $2; }
;

arithmic_unary_operator:
        '+' { TRACE $$ = ast_string("+"); }
    |   '-' { TRACE $$ = ast_string("-"); }
;

logical_unary_operator:
        '~' { TRACE $$ = ast_string("~"); }
    |   '!' { TRACE $$ = ast_string("!"); }
;


/* Things that can be used as assignment '=', '+=' etc.. */
assignment_operator:
        T_ASSIGNMENT       { TRACE $$ = T_ASSIGNMENT; }
    |   T_PLUS_ASSIGNMENT  { TRACE $$ = T_PLUS_ASSIGNMENT; }
    |   T_MINUS_ASSIGNMENT { TRACE $$ = T_MINUS_ASSIGNMENT; }
    |   T_MUL_ASSIGNMENT   { TRACE $$ = T_MUL_ASSIGNMENT; }
    |   T_DIV_ASSIGNMENT   { TRACE $$ = T_DIV_ASSIGNMENT; }
    |   T_MOD_ASSIGNMENT   { TRACE $$ = T_MOD_ASSIGNMENT; }
    |   T_AND_ASSIGNMENT   { TRACE $$ = T_AND_ASSIGNMENT; }
    |   T_OR_ASSIGNMENT    { TRACE $$ = T_OR_ASSIGNMENT; }
    |   T_XOR_ASSIGNMENT   { TRACE $$ = T_XOR_ASSIGNMENT; }
    |   T_SL_ASSIGNMENT    { TRACE $$ = T_SL_ASSIGNMENT; }
    |   T_SR_ASSIGNMENT    { TRACE $$ = T_SR_ASSIGNMENT; }
;

/* Any number, any string or null|true|false */
real_scalar_value:
        T_LNUM        { TRACE $$ = ast_numerical($1); }
    |   T_STRING      { TRACE $$ = ast_string($1); smm_free($1); }
    |   T_REGEX       { TRACE $$ = ast_string($1); smm_free($1); }
;
scalar_value:
        T_LNUM        { TRACE $$ = ast_numerical($1); }
    |   T_STRING      { TRACE $$ = ast_string($1); smm_free($1); }
    |   T_REGEX       { TRACE $$ = ast_string($1); smm_free($1); }
    |   T_IDENTIFIER  { sfc_check_permitted_identifiers($1); TRACE $$ = ast_identifier($1, ID_LOAD); smm_free($1); }
;


qualified_name:
       T_IDENTIFIER                             { TRACE $$ = ast_identifier($1, ID_LOAD); }
    |  T_NS_SEP T_IDENTIFIER                    { TRACE $$ = ast_identifier("::", ID_LOAD); $$ = ast_concat($$, $2); }
    |  qualified_name T_NS_SEP T_IDENTIFIER     { TRACE $$ = ast_concat($$, "::"); $$ = ast_concat($$, $3); }
;


method_call:
        qualified_name '.' T_IDENTIFIER     { TRACE $$ = ast_opr('.', 2, $1, ast_identifier($3, ID_LOAD)); smm_free($3); }
    |   special_name '.' T_IDENTIFIER       { TRACE $$ = ast_opr('.', 2, $1, ast_identifier($3, ID_LOAD)); smm_free($3); }

    |   method_call_pre_parenthesis '.' T_IDENTIFIER '(' calling_method_argument_list ')' { TRACE $$ = ast_opr(T_METHOD_CALL, 3, $1, ast_string($3), $5); }
    |   method_call_pre_parenthesis '.' T_IDENTIFIER '('                              ')' { TRACE $$ = ast_opr(T_METHOD_CALL, 3, $1, ast_string($3), ast_nop()); }
    |   qualified_name '.' T_IDENTIFIER '(' calling_method_argument_list ')' { TRACE $$ = ast_opr(T_METHOD_CALL, 3, $1  , ast_string($3), $5); }
    |   qualified_name '.' T_IDENTIFIER '('                              ')' { TRACE $$ = ast_opr(T_METHOD_CALL, 3, $1, ast_string($3), ast_nop()); }
    |   special_name '.' T_IDENTIFIER '(' calling_method_argument_list ')' { TRACE $$ = ast_opr(T_METHOD_CALL, 3, $1  , ast_string($3), $5); }
    |   special_name '.' T_IDENTIFIER '('                              ')' { TRACE $$ = ast_opr(T_METHOD_CALL, 3, $1, ast_string($3), ast_nop()); }
    |   qualified_name '(' calling_method_argument_list ')' { TRACE $$ = ast_opr(T_METHOD_CALL, 3, ast_nop(), $1, $3); }
    |   qualified_name '('                              ')' { TRACE $$ = ast_opr(T_METHOD_CALL, 3, ast_nop(), $1, ast_nop()); }
    |   method_call '.' T_IDENTIFIER '(' calling_method_argument_list ')' { TRACE $$ = ast_opr(T_METHOD_CALL, 3, $1, ast_string($3), $5); }
    |   method_call '.' T_IDENTIFIER '('                              ')' { TRACE $$ = ast_opr(T_METHOD_CALL, 3, $1, ast_string($3), ast_nop()); }
    |   '(' expression ')' '.' T_IDENTIFIER '(' calling_method_argument_list ')' { TRACE $$ = ast_opr(T_METHOD_CALL, 3, $2, ast_string($5), $7); smm_free($5); }
    |   '(' expression ')' '.' T_IDENTIFIER '('                              ')' { TRACE $$ = ast_opr(T_METHOD_CALL, 3, $2, ast_string($5), ast_nop()); smm_free($5); }


;

method_call_pre_parenthesis:
        T_LNUM                  { TRACE $$ = ast_numerical($1); }
    |   T_STRING                { TRACE $$ = ast_string($1); smm_free($1); }
    |   T_REGEX                 { TRACE $$ = ast_string($1); smm_free($1); }
;

/* Self and parent are processed differently, since they are separate tokens */
special_name:
        T_SELF      { TRACE $$ = ast_identifier("self", ID_LOAD); }
    |   T_PARENT    { TRACE $$ = ast_identifier("parent", ID_LOAD); }
;

/* argument list inside a method call*/
calling_method_argument_list:
        assignment_expression                                     { TRACE $$ = ast_opr(T_ARGUMENT_LIST, 1, $1); }
    |   calling_method_argument_list ',' assignment_expression    { TRACE $$ = ast_add($$, $3); }
    |   /* empty */                                               { TRACE $$ = ast_nop(); }
;

/**
 ************************************************************
 *                  CLASS PARSING
 ************************************************************
 */


/* Statements inside a class: constant and methods */
class_inner_statement_list:
        class_inner_statement                               { TRACE $$ = ast_opr(T_STATEMENTS, 1, $1); }
    |   class_inner_statement_list class_inner_statement    { TRACE $$ = ast_add($$, $2); }
;

class_inner_statement:
        constant                    { TRACE $$ = $1; }
    |   class_property_definition   { TRACE $$ = $1; }
    |   class_method_definition     { TRACE $$ = $1; }
;

/* Statements inside an interface: constant and methods */
interface_inner_statement_list:
        interface_inner_statement                                    { TRACE $$ = ast_opr(T_STATEMENTS, 1, $1); }
    |   interface_inner_statement_list interface_inner_statement     { TRACE $$ = ast_add($$, $2); }
;

interface_inner_statement:
        interface_method_definition   { TRACE $$ = $1; }
    |   interface_property_definition { TRACE $$ = $1; }
;

interface_method_definition:
    interface_or_abstract_method_definition { TRACE $$ = $1; }
;

interface_or_abstract_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' ';'   { sfc_validate_method_modifiers($1); sfc_init_method($3); sfc_fini_method(); TRACE $$ = ast_method($1, $3, $5, ast_nop()); smm_free($3); }
;

class_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' { sfc_init_method($3); sfc_validate_method_modifiers($1); } compound_statement { sfc_fini_method(); sfc_validate_abstract_method_body($1, $8); TRACE $$ = ast_method($1, $3, $5, $8); smm_free($3); }
    |   interface_or_abstract_method_definition { TRACE $$ = $1; }
;

method_argument_list:
        non_empty_method_argument_list { TRACE $$ = $1; }
    |   /* empty */                    { TRACE $$ = ast_nop(); }
;

non_empty_method_argument_list:
        method_argument                                       { TRACE $$ = ast_opr(T_ARGUMENT_LIST, 1, $1); }
    |   non_empty_method_argument_list ',' method_argument    { TRACE $$ = ast_add($$, $3); }
;

method_argument:
        T_IDENTIFIER                                           { TRACE $$ = ast_opr(T_METHOD_ARGUMENT, 4, ast_nop(), ast_identifier($1, ID_LOAD), ast_nop(), ast_numerical(0)); smm_free($1); }
    |   T_IDENTIFIER T_ASSIGNMENT scalar_value                 { TRACE $$ = ast_opr(T_METHOD_ARGUMENT, 4, ast_nop(), ast_identifier($1, ID_STORE), $3, ast_numerical(0)); smm_free($1); }
    |   T_IDENTIFIER T_IDENTIFIER                              { TRACE $$ = ast_opr(T_METHOD_ARGUMENT, 4, ast_identifier($1, ID_LOAD), ast_identifier($2, ID_LOAD), ast_nop(), ast_numerical(0)); smm_free($1); smm_free($2); }
    |   T_IDENTIFIER T_IDENTIFIER T_ASSIGNMENT scalar_value    { TRACE $$ = ast_opr(T_METHOD_ARGUMENT, 4, ast_identifier($1, ID_LOAD), ast_identifier($2, ID_STORE), $4, ast_numerical(0)); smm_free($1); smm_free($2); }
    |   T_ELLIPSIS T_IDENTIFIER                                { TRACE $$ = ast_opr(T_METHOD_ARGUMENT, 4, ast_nop(), ast_identifier($2, ID_LOAD), ast_nop(), ast_numerical(1)); smm_free($2); }
    |   T_IDENTIFIER T_ELLIPSIS T_IDENTIFIER                   { TRACE $$ = ast_opr(T_METHOD_ARGUMENT, 4, ast_nop(), ast_identifier($1, ID_LOAD), $3, ast_numerical(1)); smm_free($1); }
;

constant_list:
        constant                    { TRACE $$ = ast_opr(T_CONSTANTS, 1, $1); }
    |   constant_list constant      { TRACE $$ = ast_add($$, $1); }
;

constant:
        T_CONST T_IDENTIFIER T_ASSIGNMENT real_scalar_value ';' { TRACE sfc_validate_constant($2); $$ = ast_opr(T_CONST, 2, ast_identifier($2, ID_STORE), $4); smm_free($2); }
;

class_definition:
        class_header '{' class_inner_statement_list '}' { TRACE $$ = ast_class(global_table->active_class, $3); sfc_fini_class(); }
    |   class_header '{'                            '}' { TRACE $$ = ast_class(global_table->active_class, ast_nop()); sfc_fini_class(); }
;

class_header:
        modifier_list T_CLASS T_IDENTIFIER class_extends class_interface_implements { sfc_validate_class_modifiers($1); sfc_init_class($1, $3, $4, $5); smm_free($3); }
    |                 T_CLASS T_IDENTIFIER class_extends class_interface_implements { sfc_init_class( 0, $2, $3, $4); smm_free($2); }
;

interface_definition:
        modifier_list T_INTERFACE T_IDENTIFIER class_interface_implements '{' interface_inner_statement_list '}' { TRACE sfc_validate_class_modifiers($1); $$ = ast_interface($1, $3, $4, $6); smm_free($3); }
    |   modifier_list T_INTERFACE T_IDENTIFIER class_interface_implements '{'                                '}' { TRACE sfc_validate_class_modifiers($1); $$ = ast_interface($1, $3, $4, ast_nop()); smm_free($3); }
    |                 T_INTERFACE T_IDENTIFIER class_interface_implements '{' interface_inner_statement_list '}' { TRACE $$ = ast_interface(0, $2, $3, $5); smm_free($2); }
    |                 T_INTERFACE T_IDENTIFIER class_interface_implements '{'                                '}' { TRACE $$ = ast_interface(0, $2, $3, ast_nop()); smm_free($2); };


class_property_definition:
        modifier_list T_PROPERTY T_IDENTIFIER T_ASSIGNMENT expression ';'   { TRACE sfc_validate_property_modifiers($1); $$ = ast_opr(T_PROPERTY, 3, ast_numerical($1), ast_identifier($3, ID_STORE), $5); smm_free($3); }
    |   modifier_list T_PROPERTY T_IDENTIFIER ';'                           { TRACE sfc_validate_property_modifiers($1); $$ = ast_opr(T_PROPERTY, 2, ast_numerical($1), ast_identifier($3, ID_LOAD)); smm_free($3); }
;

interface_property_definition:
        modifier_list T_PROPERTY T_IDENTIFIER ';' { TRACE sfc_validate_property_modifiers($1); $$ = ast_opr(T_PROPERTY, 2, ast_numerical($1), ast_identifier($3, ID_LOAD)); smm_free($3); }
;

modifier_list:
        modifier               { TRACE $$ = $1; }
    |   modifier_list modifier { TRACE sfc_validate_flags($$, $2); $$ |= $2; }
;

/* Property and method modifiers. */
modifier:
        T_PROTECTED { TRACE $$ = MODIFIER_PROTECTED; }
    |   T_PUBLIC    { TRACE $$ = MODIFIER_PUBLIC; }
    |   T_PRIVATE   { TRACE $$ = MODIFIER_PRIVATE; }
    |   T_FINAL     { TRACE $$ = MODIFIER_FINAL; }
    |   T_ABSTRACT  { TRACE $$ = MODIFIER_ABSTRACT; }
    |   T_STATIC    { TRACE $$ = MODIFIER_STATIC; }
    |   T_READONLY  { TRACE $$ = MODIFIER_READONLY; }
;

/* extends only one class */
class_extends:
        T_EXTENDS T_IDENTIFIER { TRACE $$ = ast_string($2); smm_free($2); }
    |   /* empty */            { TRACE $$ = ast_nop(); }
;

/* implements a list of classes, or no implement at all */
class_interface_implements:
        T_IMPLEMENTS class_list { TRACE $$ = ast_opr(T_IMPLEMENTS, 1, $2); }
    |   /* empty */             { TRACE $$ = ast_nop(); }
;

/* Comma separated list of classes (for extends and implements) */
class_list:
        T_IDENTIFIER                { TRACE $$ = ast_opr(T_STATEMENTS, 1, ast_string($1)); smm_free($1); }
    |   class_list ',' T_IDENTIFIER { TRACE $$ = ast_add($$, ast_string($3)); smm_free($3); }
;



/**
 ************************************************************
 * data structures
 ************************************************************
 */

data_structure:
        data_structure_name  '[' data_structure_elements     ']' { TRACE $$ = ast_opr(T_DATA_STRUCTURE, 2, $1, $3); }
    |   data_structure_name  '[' data_structure_elements ',' ']' { TRACE $$ = ast_opr(T_DATA_STRUCTURE, 2, $1, $3); }
    |   data_structure_name  '[' /* empty */                 ']' { TRACE $$ = ast_opr(T_DATA_STRUCTURE, 1, $1); }

    |   method_call  '[' data_structure_elements     ']' { TRACE $$ = ast_opr(T_DATA_STRUCTURE, 2, $1, $3); }
    |   method_call  '[' data_structure_elements ',' ']' { TRACE $$ = ast_opr(T_DATA_STRUCTURE, 2, $1, $3); }
    |   method_call  '[' /* empty */                 ']' { TRACE $$ = ast_opr(T_DATA_STRUCTURE, 1, $1); }

;

data_structure_name:
        T_STRING        { TRACE $$ = ast_string($1); }
    |   qualified_name  { TRACE $$ = $1; }
;

data_structure_elements:
        data_structure_element                             { TRACE $$ = ast_opr(T_DATA_ELEMENTS, 1, $1); }
    |   data_structure_elements ',' data_structure_element { TRACE $$ = ast_add($$, $3); }
;

data_structure_element:
        assignment_expression                            { TRACE $$ = ast_opr(T_DATA_ELEMENT, 1, $1); }
    |   data_structure_element ':' assignment_expression { TRACE $$ = ast_add($$, $3); }
;

%%

   /* Returns the actual name of a token, must be implemented here, because we have no access
      to the yytoknum and yytname otherwise. */
   char *get_token_string(int token) {
        for (int i=0; i!=YYNTOKENS; i++) {
            if (token == yytoknum[i]) {
                return (char *)yytname[i];
            }
        }
        return "<unknown>";
    }
