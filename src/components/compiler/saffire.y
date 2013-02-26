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

/* Define this so we gain external access to our token defines */
#define YY_HEADER_EXPORT_START_CONDITIONS 1

    #include <stdio.h>
    #include "general/output.h"
    #include "general/smm.h"
    #include "compiler/lex.yy.h"
    #include "compiler/compiler.h"
    #include "compiler/ast.h"
    #include "objects/objects.h"
    #include "debug.h"

    extern int yylineno;
    int yylex(void);

    // Our own error manager
    void yyerror(unsigned long *ast, const char *err) {
        error_and_die(1, "line %lu: %s\n", (unsigned long)yylineno, err);
        exit(1);
    }

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
    #endif

    #define YYPRINT(file, type, value)
%}


%union {
    char                 *sVal;     // Holds any string
    long                 lVal;      // Holds any numerical value
    double               dVal;      // Holds any double value
    struct _ast_element  *nPtr;     // Holds any ast-element
}

%token END 0 "end of file"
%token <lVal> T_LNUM
%token <sVal> T_STRING T_IDENTIFIER T_REGEX

%type <lVal> modifier modifier_list assignment_operator comparison_operator

/* These must be sorted and used properly */
%token T_WHILE T_IF T_USE T_AS T_DO T_SWITCH T_FOR T_FOREACH T_CASE
%nonassoc T_ELSE
%token T_OP_INC T_OP_DEC T_PLUS_ASSIGNMENT T_MINUS_ASSIGNMENT T_MUL_ASSIGNMENT T_DIV_ASSIGNMENT
%token T_MOD_ASSIGNMENT T_AND_ASSIGNMENT T_OR_ASSIGNMENT T_XOR_ASSIGNMENT T_SL_ASSIGNMENT T_SR_ASSIGNMENT
%token T_CATCH T_BREAK T_GOTO T_BREAKELSE T_CONTINUE T_THROW T_RETURN T_FINALLY T_TRY T_DEFAULT T_METHOD
%token T_SELF T_PARENT T_NS_SEP T_TO
%left T_ASSIGNMENT T_GE T_LE T_EQ T_NE '>' '<' '^' T_IN T_RE T_REGEX
%left '+' '-'
%left '*' '/'
%token T_AND T_OR T_SHIFT_LEFT T_SHIFT_RIGHT
%token T_CLASS T_EXTENDS T_ABSTRACT T_FINAL T_IMPLEMENTS T_INTERFACE
%token T_PUBLIC T_PRIVATE T_PROTECTED T_CONST T_STATIC T_PROPERTY
%token T_LABEL T_CALL T_ARITHMIC T_LOGICAL T_PROGRAM
%token T_FQN T_ARGUMENT_LIST T_ASSIGNMENT T_CALL_ARGUMENT_LIST
%token T_MODIFIERS T_CONSTANTS T_DATA_ELEMENTS T_DATA_STRUCTURE T_DATA_ELEMENT T_METHOD_ARGUMENT
%token T_IMPORT T_FROM T_ELLIPSIS T_SUBSCRIPT T_DATASTRUCT

/* reserved for later use */
%token T_YIELD

%type <nPtr> program use_statement_list non_empty_use_statement_list use_statement top_statement_list
%type <nPtr> non_empty_top_statement_list top_statement class_definition interface_definition
%type <nPtr> statement_list compound_statement statement expression_statement jump_statement
%type <nPtr> label_statement iteration_statement class_list while_statement
%type <nPtr> guarding_statement expression catch_list catch ds_element ds_elements
%type <nPtr> class_interface_implements method_argument_list interface_or_abstract_method_definition class_extends
%type <nPtr> non_empty_method_argument_list interface_inner_statement_list class_inner_statement class_inner_statement_list
%type <nPtr> if_statement switch_statement class_constant_definition
%type <nPtr> unary_expression primary_expression pe_no_parenthesis data_structure
%type <nPtr> logical_unary_operator multiplicative_expression additive_expression shift_expression regex_expression
%type <nPtr> catch_header conditional_expression assignment_expression real_scalar_value
%type <nPtr> method_argument interface_inner_statement interface_method_definition interface_property_definition
%type <nPtr> class_method_definition class_property_definition qualified_name calling_method_argument_list
%type <nPtr> logical_unary_expression equality_expression and_expression inclusive_or_expression
%type <nPtr> conditional_or_expression exclusive_or_expression conditional_and_expression case_statements case_statement
%type <nPtr> special_name scalar_value callable var_callable subscription primary_expression_first_part qualified_name_first_part

%type <sVal> T_ASSIGNMENT T_PLUS_ASSIGNMENT T_MINUS_ASSIGNMENT T_MUL_ASSIGNMENT T_DIV_ASSIGNMENT T_MOD_ASSIGNMENT T_AND_ASSIGNMENT
%type <sVal> T_OR_ASSIGNMENT T_XOR_ASSIGNMENT T_SL_ASSIGNMENT T_SR_ASSIGNMENT '~' '!' '+' '-' T_SELF T_PARENT


/* Add token table, so we can convert a token(numerical) into it's name (330 => T_TOKEN for example) */
%token_table

/* Verbose errors */
%error-verbose

/* Define our start rule */
%start saffire

/* Parser uses ast_root as a parameter */
%parse-param { unsigned long *ast_root }



%% /* rules */


/**
 ************************************************************
 *                  TOP AND USE STATEMENTS
 ************************************************************
 */

saffire:
        /* We must convert our ast_root address to an unsigned long */
        program { *ast_root = (unsigned long)$1; }
;

program:
        /* A program consists of use-statements and a list of top-statements */
        use_statement_list top_statement_list { $$ = ast_opr(T_PROGRAM,2, $1, $2); }
;

use_statement_list:
        non_empty_use_statement_list { $$ = $1; }
    |   /* empty */                  { $$ = ast_group(0); }
;

/* A use-statement list with at least one use statement */
non_empty_use_statement_list:
        use_statement { $$ = ast_group(1, $1); }
    |   non_empty_use_statement_list use_statement { $$ = ast_add($$, $2); }
;

use_statement:
        /* use <foo> as <bar>; */
        T_USE qualified_name T_AS T_IDENTIFIER                        ';' { $$ = ast_opr(T_USE, 2, $2, ast_string($4));  }
        /* use <foo>; */
    |   T_USE qualified_name                                          ';' { $$ = ast_opr(T_USE, 1, $2); }
        /* import <foo> from <bar> */
    |   T_IMPORT T_IDENTIFIER                   T_FROM qualified_name ';' { $$ = ast_opr(T_IMPORT, 3, ast_string($2), ast_string($2), ast_string_dup($4)); }
        /* import <foo> as <bar> from <baz> */
    |   T_IMPORT T_IDENTIFIER T_AS T_IDENTIFIER T_FROM qualified_name ';' { $$ = ast_opr(T_IMPORT, 3, ast_string($2), ast_string($4), ast_string_dup($6)); }
        /* import <foo> as <bar> */
    |   T_IMPORT T_IDENTIFIER T_AS T_IDENTIFIER                       ';' { $$ = ast_opr(T_IMPORT, 3, ast_string($2), ast_string($4), ast_string($2)); }
        /* import <foo> */
    |   T_IMPORT T_IDENTIFIER                                         ';' { $$ = ast_opr(T_IMPORT, 3, ast_string($2), ast_string($2), ast_string($2)); }
;

/* Top statements are single (global) statements and/or class/interface/constant */
top_statement_list:
        non_empty_top_statement_list { $$ = $1; }
    |   /* empty */                  { $$ = ast_group(0); }
;

/* A top-statement list with at least one top statement */
non_empty_top_statement_list:
        top_statement                              { $$ = ast_group(1, $1); }
    |   non_empty_top_statement_list top_statement { $$ = ast_add($$, $2); }
;

/* Top statements can be classes, interfaces, constants, statements */
top_statement:
        class_definition        { $$ = $1; }
    |   interface_definition    { $$ = $1; }
/*    |   constant                { $$ = $1; } */
    |   statement               { $$ = $1; }  /* statement, not statementlist, since top_statement is a list already! */
;


/**
 ************************************************************
 *                 BLOCKS & STATEMENTS
 ************************************************************
 */

statement_list:
        statement                { $$ = ast_group(1, $1); }
    |   statement_list statement { $$ = ast_add($$, $2); }
;

statement:
        label_statement        { $$ = $1; }
    |   compound_statement     { $$ = $1; }
    |   jump_statement         { $$ = $1; }
    |   expression_statement   { $$ = $1; }
    |   if_statement           { $$ = $1; }
    |   switch_statement       { $$ = $1; }
    |   iteration_statement    { $$ = $1; }
    |   guarding_statement     {  $$ = $1; }
;

/* A compound statement is a (set of) statement captured by curly brackets */
compound_statement:
        '{'                '}'  { $$ = ast_nop(); }
    |   '{' statement_list '}'  { $$ = $2; }
;

/* if if/else statements */
if_statement:
        T_IF '(' conditional_expression ')' statement                  { $$ = ast_opr(T_IF, 2, $3, $5); }
    |   T_IF '(' conditional_expression ')' statement T_ELSE statement { $$ = ast_opr(T_IF, 3, $3, $5, $7); }
;

/* Switch statement */
switch_statement:
        T_SWITCH '(' conditional_expression ')' { sfc_loop_enter(); sfc_switch_begin(); } '{' case_statements '}' { sfc_loop_leave(); sfc_switch_end();  $$ = ast_opr(T_SWITCH, 2, $3, $7); }
;

case_statements:
        case_statement                 { $$ = ast_group(1, $1); }
    |   case_statements case_statement { $$ = ast_add($$, $2); }
;

case_statement:
        T_CASE conditional_expression ':'   { sfc_switch_case(); }    statement_list { $$ = ast_opr(T_CASE, 2, $2, $5); }
    |   T_CASE conditional_expression ':'   { sfc_switch_case(); }    { $$ = ast_opr(T_CASE, 2, $2, ast_nop()); }
    |   T_DEFAULT ':'           { sfc_switch_default(); } statement_list { $$ = ast_opr(T_DEFAULT, 1, $4); }
    |   T_DEFAULT ':'           { sfc_switch_default(); } { $$ = ast_opr(T_DEFAULT, 1, ast_nop()); }
;


/* while, while else, do/while, for and foreach */
iteration_statement:
        while_statement T_ELSE statement { $$ = ast_add($1, $3); }
    |   while_statement                  { $$ = $1; }
    |   T_DO { sfc_loop_enter(); } statement T_WHILE '(' conditional_expression ')' ';' { sfc_loop_leave();  $$ = ast_opr(T_DO, 2, $3, $6); }
    |   T_FOR '(' expression_statement expression_statement            ')' { sfc_loop_enter(); } statement { $$ = ast_opr(T_FOR, 3, $3, $4, $7); }
    |   T_FOR '(' expression_statement expression_statement expression ')' { sfc_loop_enter(); } statement { $$ = ast_opr(T_FOR, 4, $3, $4, $5, $8); }
    |   T_FOREACH '(' expression T_AS ds_element                       ')' { sfc_loop_enter(); } statement { sfc_loop_leave();  $$ = ast_opr(T_FOREACH, 2, $3, $5); }
    |   T_FOREACH '(' expression T_AS ds_element ',' T_IDENTIFIER      ')' { sfc_loop_enter(); } statement { sfc_loop_leave();  $$ = ast_opr(T_FOREACH, 3, $3, $5, $7); }
;

/* while is separate otherwise we cannot find it's else */
while_statement:
        T_WHILE '(' conditional_expression ')' { sfc_loop_enter(); } statement { sfc_loop_leave();  $$ = ast_opr(T_WHILE, 2, $3, $6); }
;

/* An expression is anything that evaluates something */
expression_statement:
        ';'             { $$ = ast_opr(';', 0); }
    |   expression ';'  { $$ = $1; }
;


/* Jumps to another part of the code */
jump_statement:
        T_BREAK ';'                 { saffire_validate_break();  $$ = ast_opr(T_BREAK, 0); }
    |   T_BREAKELSE ';'             { saffire_validate_breakelse();  $$ = ast_opr(T_BREAKELSE, 0); }
    |   T_CONTINUE ';'              { saffire_validate_continue();  $$ = ast_opr(T_CONTINUE, 0); }
    |   T_RETURN ';'                { saffire_validate_return();  $$ = ast_opr(T_RETURN, 1, ast_identifier("null")); }
    |   T_RETURN expression ';'     { saffire_validate_return();  $$ = ast_opr(T_RETURN, 1, $2); }
    |   T_THROW expression ';'      { $$ = ast_opr(T_THROW, 1, $2); }
    |   T_GOTO T_IDENTIFIER ';'     { $$ = ast_opr(T_GOTO, 1, ast_string($2)); smm_free($2); }
    |   T_GOTO T_LNUM ';'           { $$ = ast_opr(T_GOTO, 1, ast_numerical($2)); }                 // @TODO: Should support goto 3; ?
;

/* try/catch try/catch/finally blocks */
guarding_statement:
        T_TRY compound_statement catch_list                               { $$ = ast_opr(T_TRY, 3, $2, $3, ast_nop()); }
    |   T_TRY compound_statement catch_list T_FINALLY compound_statement  { $$ = ast_opr(T_TRY, 3, $2, $3, $5); }
;

/* There can be multiple catches on a try{} block */
catch_list:
        catch            { $$ = ast_group(1, $1); }
    |   catch_list catch { $$ = ast_add($$, $2); }
;

catch:
        catch_header compound_statement { $$ = ast_group(2, $1, $2); }
;

catch_header:
        T_CATCH '(' T_IDENTIFIER T_IDENTIFIER ')' { $$ = ast_opr(T_CATCH, 2, ast_identifier($3), ast_identifier($4)); smm_free($3); smm_free($4); }
;

label_statement:
        T_IDENTIFIER ':'    { saffire_check_label($1);  $$ = ast_opr(T_LABEL, 1, ast_string($1)); smm_free($1); }
    |   T_LNUM ':'          { $$ = ast_opr(T_LABEL, 1, ast_numerical($1)); }  /* @TODO: should we support goto 4; ? */
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
        assignment_expression { $$ = $1; }
    |   expression ',' assignment_expression { $$ = ast_add($$, $3); }
    |   expression ',' ',' assignment_expression { $$ = ast_add($$, ast_nop()); $$ = ast_add($$, $4); }
;

assignment_expression:
        conditional_expression { $$ = $1; }
    |   unary_expression assignment_operator assignment_expression { $$ = ast_assignment($2, $1, $3); }
;

conditional_expression:
        conditional_or_expression { $$ = $1; }
    |   conditional_or_expression '?' expression ':' conditional_expression { $$ = ast_opr('?', 2, $1, $3); }
;

conditional_or_expression:
        conditional_and_expression { $$ = $1; }
    |   conditional_or_expression T_OR conditional_and_expression { $$ = ast_boolop(1, $1, $3);}
;

conditional_and_expression:
        inclusive_or_expression { $$ = $1; }
    |   conditional_and_expression T_AND inclusive_or_expression { $$ = ast_boolop(0, $1, $3); }
;

inclusive_or_expression:
        exclusive_or_expression { $$ = $1; }
    |   inclusive_or_expression '|' exclusive_or_expression { $$ = ast_opr('|', 2, $1, $3);}
;

exclusive_or_expression:
        and_expression { $$ = $1; }
    |   exclusive_or_expression '^' and_expression { $$ = ast_opr('^', 2, $1, $3); }
;

and_expression:
        equality_expression { $$ = $1; }
    |   and_expression '&' equality_expression { $$ = ast_opr('&', 2, $1, $3); }
;

equality_expression:
        regex_expression { $$ = $1; }
    |   equality_expression comparison_operator regex_expression { $$ = ast_comparison($2, $1, $3); }
;

comparison_operator:
        T_EQ { $$ = T_EQ; }
    |   T_NE { $$ = T_NE; }
    |   T_IN { $$ = T_IN; }
    |   '>'  { $$ = '>';  }
    |   '<'  { $$ = '<';  }
    |   T_LE { $$ = T_LE; }
    |   T_GE { $$ = T_GE; }
;

regex_expression:
        shift_expression { $$ = $1; }
    |   regex_expression T_RE T_REGEX { $$ = ast_opr(T_RE, 2, $1, ast_string($3)); smm_free($3); }
;

shift_expression:
        additive_expression { $$ = $1; }
    |   shift_expression T_SHIFT_LEFT additive_expression { $$ = ast_operator(T_SHIFT_LEFT, $1, $3); }
    |   shift_expression T_SHIFT_RIGHT additive_expression { $$ = ast_operator(T_SHIFT_RIGHT, $1, $3); }
;

additive_expression:
        multiplicative_expression { $$ = $1; }
    |   additive_expression '+' multiplicative_expression { $$ = ast_operator('+', $1, $3); }
    |   additive_expression '-' multiplicative_expression { $$ = ast_operator('-', $1, $3); }
;

multiplicative_expression:
        unary_expression { $$ = $1; }
    |   multiplicative_expression '*' unary_expression { $$ = ast_operator('*', $1, $3); }
    |   multiplicative_expression '/' unary_expression { $$ = ast_operator('/', $1, $3); }
    |   multiplicative_expression '%' unary_expression { $$ = ast_operator('%', $1, $3); }
;

unary_expression:
        logical_unary_expression                    { $$ = $1; }
    |   T_OP_INC unary_expression                   { $$ = ast_operator('+', $2, ast_numerical(1)); }
    |   T_OP_DEC unary_expression                   { $$ = ast_operator('-', $2, ast_numerical(1)); }
;

logical_unary_expression:
        primary_expression                      { $$ = $1; }
    |   logical_unary_operator unary_expression { $$ = ast_opr(T_LOGICAL, 2, $1, $2); }
;

logical_unary_operator:
        '~' { $$ = ast_string("~"); }
    |   '!' { $$ = ast_string("!"); }
;

/* Things that can be used as assignment '=', '+=' etc.. */
assignment_operator:
        T_ASSIGNMENT       { $$ = T_ASSIGNMENT; }
    |   T_PLUS_ASSIGNMENT  { $$ = T_PLUS_ASSIGNMENT; }
    |   T_MINUS_ASSIGNMENT { $$ = T_MINUS_ASSIGNMENT; }
    |   T_MUL_ASSIGNMENT   { $$ = T_MUL_ASSIGNMENT; }
    |   T_DIV_ASSIGNMENT   { $$ = T_DIV_ASSIGNMENT; }
    |   T_MOD_ASSIGNMENT   { $$ = T_MOD_ASSIGNMENT; }
    |   T_AND_ASSIGNMENT   { $$ = T_AND_ASSIGNMENT; }
    |   T_OR_ASSIGNMENT    { $$ = T_OR_ASSIGNMENT; }
    |   T_XOR_ASSIGNMENT   { $$ = T_XOR_ASSIGNMENT; }
    |   T_SL_ASSIGNMENT    { $$ = T_SL_ASSIGNMENT; }
    |   T_SR_ASSIGNMENT    { $$ = T_SR_ASSIGNMENT; }
;

/**
 ************************************************************
 *                  NAMING THINGS
 ************************************************************
 */

/* Any number, any string, any regex */
real_scalar_value:
        T_LNUM        { $$ = ast_numerical($1); }
    |   T_STRING      { $$ = ast_string($1); smm_free($1); }
    |   T_REGEX       { $$ = ast_string($1); smm_free($1); }
;

/* Any number, any string, any regex or null|true|false */
scalar_value:
        real_scalar_value   { $$ = $1; }
    |   T_IDENTIFIER        { sfc_check_permitted_identifiers($1);  $$ = ast_identifier($1); smm_free($1); }
;

/* This is primary expression */
primary_expression:
        pe_no_parenthesis     { $$ = $1; }
   |    '(' expression ')'    { $$ = $2; }
;

pe_no_parenthesis:
        primary_expression_first_part        { $$ = $1; }
    |   primary_expression '.' T_IDENTIFIER  { $$ = ast_property($1, ast_string($3)); }
    |   primary_expression callable          { $$ = ast_opr(T_CALL, 2, $1, ast_add($2, ast_null())); } /* Add termination varargs list */
    |   primary_expression var_callable      { $$ = ast_opr(T_CALL, 2, $1, $2); }
    |   primary_expression subscription      { $$ = ast_opr(T_SUBSCRIPT, 2, $1, $2); }
    |   primary_expression data_structure    { $$ = ast_opr(T_DATASTRUCT, 2, $1, $2); }
    |   primary_expression T_OP_INC          { $$ = ast_operator('+', $1, ast_numerical(1)); }
    |   primary_expression T_OP_DEC          { $$ = ast_operator('-', $1, ast_numerical(1)); }
;

/* First part is different (can be namespaced / ++foo / --foo etc */
primary_expression_first_part:
        qualified_name          { $$ = $1; }   /* fully qualified name */
    |   special_name            { $$ = $1; }   /* self or parent */
    |   real_scalar_value       { $$ = $1; }   /* digits, strings, regexes */
;

/* A name that is namespaced (or not). */
qualified_name:
        qualified_name_first_part               { $$ = $1; }
    |   qualified_name T_NS_SEP T_IDENTIFIER    { $$ = ast_concat($1, "::"); $$ = ast_concat($$, $3); }
;

qualified_name_first_part:
        T_IDENTIFIER            { $$ = ast_identifier($1); }
    |   T_NS_SEP T_IDENTIFIER   { $$ = ast_string("::"); $$ = ast_concat($$, $2); }
;


/* Self and parent are processed differently, since they are separate tokens */
special_name:
        T_SELF    { $$ = ast_identifier("self"); }
    |   T_PARENT  { $$ = ast_identifier("parent"); }
;

var_callable:
        '(' calling_method_argument_list ',' T_ELLIPSIS expression ')' { $$ = ast_add($2, $5); }
    |   '(' T_ELLIPSIS expression                                  ')' { $$ = ast_group(1, $3); }
;

callable:
        '(' calling_method_argument_list ')' { $$ = $2; }
    |   '(' /* empty */                  ')' { $$ = ast_group(0); }
;

/* argument list inside a method call*/
calling_method_argument_list:
        assignment_expression                                     { $$ = ast_group(1, $1); }
    |   calling_method_argument_list ',' assignment_expression    { $$ = ast_add($$, $3); }
;

data_structure:
        '[' ds_elements ']' { $$ = $2; }
;

subscription:
        '[' pe_no_parenthesis T_TO                                   ']' { $$ = ast_group(2, $2, ast_null()); }
    |   '[' pe_no_parenthesis T_TO pe_no_parenthesis ']' { $$ = ast_group(2, $2, $4); }
    |   '['                   T_TO pe_no_parenthesis ']' { $$ = ast_group(2, ast_null(), $3); }
    |   '[' /* empty */                ']' { $$ = ast_group(2, ast_null(), ast_null()); }
;

/**
 ************************************************************
 *                  CLASS PARSING
 ************************************************************
 */


/* Statements inside a class: constant and methods */
class_inner_statement_list:
        class_inner_statement                               { $$ = ast_group(1, $1); }
    |   class_inner_statement_list class_inner_statement    { $$ = ast_add($$, $2); }
;

class_inner_statement:
        class_constant_definition   { $$ = $1; }
    |   class_property_definition   { $$ = $1; }
    |   class_method_definition     { $$ = $1; }
;

/* Statements inside an interface: constant and methods */
interface_inner_statement_list:
        interface_inner_statement                                    { $$ = ast_group(1, $1); }
    |   interface_inner_statement_list interface_inner_statement     { $$ = ast_add($$, $2); }
;

interface_inner_statement:
        interface_method_definition   { $$ = $1; }
    |   interface_property_definition { $$ = $1; }
;

interface_method_definition:
    interface_or_abstract_method_definition { $$ = $1; }
;

interface_or_abstract_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' ';'   { sfc_validate_method_modifiers($1); sfc_init_method($3); sfc_fini_method();  $$ = ast_attribute($3, ATTRIB_TYPE_METHOD, sfc_mod_to_visibility($1), ATTRIB_ACCESS_RO, ast_nop(), sfc_mod_to_methodflags($1), $5); smm_free($3); }
;

class_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' { sfc_init_method($3); sfc_validate_method_modifiers($1); } compound_statement { sfc_fini_method(); sfc_validate_abstract_method_body($1, $8);  $$ = ast_attribute($3, ATTRIB_TYPE_METHOD, sfc_mod_to_visibility($1), ATTRIB_ACCESS_RO, $8, sfc_mod_to_methodflags($1), $5); smm_free($3); }
    |   interface_or_abstract_method_definition { $$ = $1; }
;

method_argument_list:
        non_empty_method_argument_list                             { $$ = $1; }
    |   /* empty */                                                { $$ = ast_nop(); }
    |   non_empty_method_argument_list ',' T_ELLIPSIS T_IDENTIFIER { $$ = $1; ast_add($$, ast_opr(T_METHOD_ARGUMENT, 3, ast_string("..."), ast_string($4), ast_null())); smm_free($4); }
    |   /* empty */                        T_ELLIPSIS T_IDENTIFIER { $$ = ast_opr(T_ARGUMENT_LIST, 0); ast_add($$, ast_opr(T_METHOD_ARGUMENT, 3, ast_string("..."), ast_string($2), ast_null())); smm_free($2); }
;

non_empty_method_argument_list:
        method_argument                                       { $$ = ast_opr(T_ARGUMENT_LIST, 1, $1); }
    |   non_empty_method_argument_list ',' method_argument    { $$ = ast_add($$, $3); }
;

method_argument:
        T_IDENTIFIER                                           { $$ = ast_opr(T_METHOD_ARGUMENT, 3, ast_null(),     ast_string($1), ast_null()); smm_free($1); }
    |   T_IDENTIFIER T_ASSIGNMENT scalar_value                 { $$ = ast_opr(T_METHOD_ARGUMENT, 3, ast_null(),     ast_string($1), $3        ); smm_free($1); }
    |   T_IDENTIFIER T_IDENTIFIER                              { $$ = ast_opr(T_METHOD_ARGUMENT, 3, ast_string($1), ast_string($2), ast_null()); smm_free($1); smm_free($2); }
    |   T_IDENTIFIER T_IDENTIFIER T_ASSIGNMENT scalar_value    { $$ = ast_opr(T_METHOD_ARGUMENT, 3, ast_string($1), ast_string($2), $4        ); smm_free($1); smm_free($2); }
;

class_definition:
        class_header '{' class_inner_statement_list '}' { $$ = ast_class(global_table->active_class, $3); sfc_fini_class(); }
    |   class_header '{'                            '}' { $$ = ast_class(global_table->active_class, ast_nop()); sfc_fini_class(); }
;

class_header:
        modifier_list T_CLASS T_IDENTIFIER class_extends class_interface_implements { sfc_validate_class_modifiers($1); sfc_init_class(sfc_mod_to_methodflags($1), $3, $4, $5); smm_free($3); }
    |                 T_CLASS T_IDENTIFIER class_extends class_interface_implements { sfc_init_class( 0, $2, $3, $4); smm_free($2); }
;

interface_definition:
        modifier_list T_INTERFACE T_IDENTIFIER class_interface_implements '{' interface_inner_statement_list '}' { sfc_validate_class_modifiers($1); $$ = ast_interface(sfc_mod_to_methodflags($1), $3, $4, $6); smm_free($3); }
    |   modifier_list T_INTERFACE T_IDENTIFIER class_interface_implements '{'                                '}' { sfc_validate_class_modifiers($1); $$ = ast_interface(sfc_mod_to_methodflags($1), $3, $4, ast_nop()); smm_free($3); }
    |                 T_INTERFACE T_IDENTIFIER class_interface_implements '{' interface_inner_statement_list '}' { $$ = ast_interface(0, $2, $3, $5); smm_free($2); }
    |                 T_INTERFACE T_IDENTIFIER class_interface_implements '{'                                '}' { $$ = ast_interface(0, $2, $3, ast_nop()); smm_free($2); };
;

class_property_definition:
        modifier_list T_PROPERTY T_IDENTIFIER T_ASSIGNMENT expression ';'   { sfc_validate_property_modifiers($1); $$ = ast_attribute($3, ATTRIB_TYPE_PROPERTY, sfc_mod_to_visibility($1), ATTRIB_ACCESS_RW, $5, 0, ast_nop()); smm_free($3); }
    |   modifier_list T_PROPERTY T_IDENTIFIER ';'                           { sfc_validate_property_modifiers($1); $$ = ast_attribute($3, ATTRIB_TYPE_PROPERTY, sfc_mod_to_visibility($1), ATTRIB_ACCESS_RW, ast_null(), 0, ast_nop()); smm_free($3); }
;

class_constant_definition:
        modifier_list T_CONST T_IDENTIFIER T_ASSIGNMENT scalar_value ';' { sfc_validate_property_modifiers($1); $$ = ast_attribute($3, ATTRIB_TYPE_PROPERTY, sfc_mod_to_visibility($1), ATTRIB_ACCESS_RO, $5, 0, ast_nop()); smm_free($3); }
;


interface_property_definition:
        modifier_list T_PROPERTY T_IDENTIFIER ';' { sfc_validate_property_modifiers($1); $$ = ast_opr(T_PROPERTY, 2, sfc_mod_to_visibility($1), ast_identifier($3)); smm_free($3); }
;

modifier_list:
        modifier               { $$ = $1; }
    |   modifier_list modifier { sfc_validate_flags($$, $2); $$ |= $2; }
;

/* Property and method modifiers. */
modifier:
        T_PROTECTED { $$ = MODIFIER_PROTECTED; }
    |   T_PUBLIC    { $$ = MODIFIER_PUBLIC; }
    |   T_PRIVATE   { $$ = MODIFIER_PRIVATE; }
    |   T_FINAL     { $$ = MODIFIER_FINAL; }
    |   T_ABSTRACT  { $$ = MODIFIER_ABSTRACT; }
    |   T_STATIC    { $$ = MODIFIER_STATIC; }
;

/* extends only one class */
class_extends:
        T_EXTENDS T_IDENTIFIER { $$ = ast_string($2); smm_free($2); }
    |   /* empty */            { $$ = ast_null(); }
;

/* implements a list of classes, or no implement at all */
class_interface_implements:
        T_IMPLEMENTS class_list { $$ = $2; }
    |   /* empty */             { $$ = ast_group(0); }
;

/* Comma separated list of classes (for extends and implements) */
class_list:
        T_IDENTIFIER                { $$ = ast_group(1, ast_string($1)); smm_free($1); }
    |   class_list ',' T_IDENTIFIER { $$ = ast_add($$, ast_string($3)); smm_free($3); }
;



/**
 ************************************************************
 * data structures
 ************************************************************
 */

ds_elements:
        ds_element                 { $$ = ast_group(1, $1); }
    |   ds_elements ',' ds_element { $$ = ast_add($$, $3); }
;

ds_element:
        assignment_expression                { $$ = ast_group(1, $1); }
    |   ds_element ':' assignment_expression { $$ = ast_add($$, $3); }
;

%%

   /* Returns the actual name of a token, must be implemented here, because we have no access
      to the yytoknum and yytname otherwise in other source files. */
   char *get_token_string(int token) {
        for (int i=0; i!=YYNTOKENS; i++) {
            if (token == yytoknum[i]) {
                return (char *)yytname[i];
            }
        }
        return "<unknown>";
    }
