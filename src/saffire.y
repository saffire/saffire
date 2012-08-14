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
%{


#define YY_HEADER_EXPORT_START_CONDITIONS 1

    #include <stdio.h>
    #include "ast.h"
    #include "saffire_parser.h"
    #include "lex.yy.h"

    extern int yylineno;
    int yylex(void);
    void yyerror(const char *err) { printf("Error in line: %d: %s\n", yylineno, err); }

    void saffire_push_state(int state);
    void saffire_pop_state();

    #ifdef __DEBUG
        #define YYDEBUG 1
        #define TRACE printf("Reduce at line %d\n", __LINE__);
    #else
        #define YYDEBUG 0
        #define TRACE
    #endif

%}

%union {
    char *sVal;
    long lVal;
    double dVal;
    t_ast_element *nPtr;
}

%token END 0 "end of file"
%token <lVal> T_LNUM
%token <sVal> T_STRING
%token <sVal> T_IDENTIFIER

%token T_WHILE T_IF T_USE T_AS T_DO T_SWITCH T_FOR T_FOREACH T_CASE
%nonassoc T_ELSE

%token T_OP_INC T_OP_DEC T_PLUS_ASSIGNMENT T_MINUS_ASSIGNMENT T_MUL_ASSIGNMENT T_DIV_ASSIGNMENT
%token T_MOD_ASSIGNMENT T_AND_ASSIGNMENT T_OR_ASSIGNMENT T_XOR_ASSIGNMENT T_SL_ASSIGNMENT T_SR_ASSIGNMENT

%token T_CATCH T_BREAK T_GOTO T_BREAKELSE T_CONTINUE T_THROW T_RETURN T_FINALLY T_TRY T_DEFAULT T_METHOD
%token T_SELF T_PARENT
%left '=' T_GE T_LE T_EQ T_NE '>' '<' '^' T_IN T_RE T_REGEX
%left '+' '-'
%left '*' '/'
%token T_AND T_OR T_SHIFT_LEFT T_SHIFT_RIGHT
%token T_CLASS T_EXTENDS T_ABSTRACT T_FINAL T_IMPLEMENTS T_INTERFACE
%token T_PUBLIC T_PRIVATE T_PROTECTED T_CONST T_STATIC T_READONLY T_PROPERTY

%type <nPtr> program use_statement_list non_empty_use_statement_list use_statement top_statement_list
%type <nPtr> non_empty_top_statement_list top_statement class_definition interface_definition
%type <nPtr> constant_list statement_list compound_statement statement expression_statement jump_statement
%type <nPtr> label_statement selection_statement iteration_statement
%type <nPtr> guarding_statement expression

%error-verbose

%start saffire

%% /* rules */


/**
 ************************************************************
 *                  TOP AND USE STATEMENTS
 ************************************************************
 */

saffire:
        program { TRACE saffire_execute($1); saffire_free_node($1); }
;

program:
        use_statement_list top_statement_list { TRACE $$ = saffire_opr(';',2, $1, $2); }
;

use_statement_list:
        non_empty_use_statement_list { TRACE $$ = $1; }
    |   /* empty */ { TRACE }
;

non_empty_use_statement_list:
        use_statement { TRACE $$ = $1; }
    |   non_empty_use_statement_list use_statement { TRACE $$ = saffire_opr(';', 2, $1, $2); }
;

use_statement:
        /* use <foo> as <bar>; */
        T_USE T_IDENTIFIER T_AS T_IDENTIFIER ';' { TRACE $$ = saffire_opr(T_USE, 2, saffire_strCon($2), saffire_strCon($4));}
        /* use <foo>; */
    |   T_USE T_IDENTIFIER ';' { TRACE $$ = saffire_opr(T_USE, 2, saffire_strCon($2), saffire_strCon($2)); }
;


/* Top statements are single (global) statements and/or class/interface/constant */
top_statement_list:
        non_empty_top_statement_list { TRACE $$ = $1; }
    |   /* empty */ { }
;

non_empty_top_statement_list:
        top_statement{ TRACE $$ = $1; }
    |   non_empty_top_statement_list top_statement { TRACE $$ = saffire_opr(';', 2, $1, $2); }
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


/* A compound statement is a (set of) statement captured by curly brackets */
compound_statement:
        '{' '}'                 { TRACE }
    |   '{' statement_list '}'  { TRACE $$ = $2; }
;

statement_list:
        statement                { TRACE $$ = $1; }
    |   statement_list statement { TRACE $$ = saffire_opr(';', 2, $1, $2); }
;

statement:
        label_statement        { TRACE $$ = $1; }
    |   compound_statement     { TRACE $$ = $1; }
    |   expression_statement   { TRACE $$ = $1; }
    |   selection_statement    { TRACE $$ = $1; }
    |   iteration_statement    { TRACE $$ = $1; }
    |   jump_statement         { TRACE $$ = $1; }
    |   guarding_statement     { TRACE $$ = $1; }
;

selection_statement:
        T_IF expression statement { TRACE }
    |   T_IF expression statement T_ELSE statement { TRACE }
    |   T_SWITCH '(' expression ')' compound_statement { TRACE }
;

iteration_statement:
        T_WHILE expression statement T_ELSE statement { TRACE }
    |   T_WHILE expression statement { TRACE }
    |   T_DO statement T_WHILE expression ';' { TRACE }
    |   T_FOR '(' expression_statement expression_statement ')' statement { TRACE }
    |   T_FOR '(' expression_statement expression_statement expression ')' statement { TRACE }
    |   T_FOREACH expression T_AS expression statement { TRACE }
;

expression_statement:
        ';'             { TRACE $$ = saffire_opr(';', 2, NULL, NULL); }
    |   expression ';'  { TRACE $$ = $1; }
;

jump_statement:
        T_BREAK ';'                 { TRACE $$ = saffire_opr(T_BREAK, 2, NULL, NULL); }
    |   T_BREAKELSE ';'             { TRACE $$ = saffire_opr(T_BREAKELSE, 2, NULL, NULL); }
    |   T_CONTINUE ';'              { TRACE $$ = saffire_opr(T_CONTINUE, 2, NULL, NULL); }
    |   T_RETURN ';'                { TRACE $$ = saffire_opr(T_RETURN, 2, NULL, NULL); }
    |   T_RETURN expression ';'     { TRACE $$ = saffire_opr(T_RETURN, 2, $2, NULL); }
    |   T_THROW expression ';'      { TRACE $$ = saffire_opr(T_THROW, 2, $2, NULL); }
    |   T_GOTO T_IDENTIFIER ';'     { TRACE $$ = saffire_opr(T_GOTO, 2, saffire_strCon($2), NULL); }
;

guarding_statement:
        T_TRY compound_statement catch_list                               { TRACE }
    |   T_TRY compound_statement catch_list T_FINALLY compound_statement  { TRACE }
    |   T_TRY compound_statement            T_FINALLY compound_statement  { TRACE }
;

catch_list:
        catch            { TRACE }
    |   catch_list catch { TRACE }
;

catch:
        catch_header compound_statement { TRACE }
;

catch_header:
        T_CATCH '(' T_IDENTIFIER T_IDENTIFIER ')' { TRACE }
    |   T_CATCH '('              T_IDENTIFIER ')' { TRACE }
    |   T_CATCH '('                           ')' { TRACE }
;

label_statement:
        T_IDENTIFIER ':'                { TRACE }
    |   T_CASE constant_expression ':'  { TRACE }
    |   T_DEFAULT ':'                   { TRACE }
;


/**
 ************************************************************
 *                  ASSIGNMENT & EXPRESSION
 ************************************************************
 */

constant_expression:
        conditional_expression { TRACE }
;

expression:
        assignment_expression { TRACE }
    |   expression ',' assignment_expression { TRACE }
;

assignment_expression:
        conditional_expression { TRACE }
    |   unary_expression assignment_operator assignment_expression { TRACE }
;

conditional_expression:
        conditional_or_expression { TRACE }
    |   conditional_or_expression '?' expression ':' conditional_expression { TRACE }
;

conditional_or_expression:
        conditional_and_expression { TRACE }
    |   conditional_or_expression T_OR conditional_and_expression { TRACE }
;

conditional_and_expression:
        inclusive_or_expression { TRACE }
    |   conditional_and_expression T_AND inclusive_or_expression { TRACE }
;

inclusive_or_expression:
        exclusive_or_expression { TRACE }
    |   inclusive_or_expression '|' exclusive_or_expression { TRACE }
;

exclusive_or_expression:
        and_expression { TRACE }
    |   exclusive_or_expression '^' and_expression { TRACE }
;

and_expression:
        equality_expression { TRACE }
    |   and_expression '&' equality_expression { TRACE }
;

equality_expression:
        relational_expression { TRACE }
    |   equality_expression T_EQ relational_expression { TRACE }
    |   equality_expression T_NE relational_expression { TRACE }
    |   equality_expression T_IN relational_expression { TRACE }
;

relational_expression:
        regex_expression { TRACE }
    |   relational_expression '>' regex_expression { TRACE }
    |   relational_expression '<' regex_expression { TRACE }
    |   relational_expression T_LE regex_expression { TRACE }
    |   relational_expression T_GE regex_expression { TRACE }
;

regex_expression:
        shift_expression { TRACE }
    |   regex_expression T_RE T_REGEX { TRACE }
;

shift_expression:
        additive_expression { TRACE }
    |   shift_expression T_SHIFT_LEFT additive_expression { TRACE }
    |   shift_expression T_SHIFT_RIGHT additive_expression { TRACE }
;

additive_expression:
        multiplicative_expression { TRACE }
    |   additive_expression '+' multiplicative_expression { TRACE }
    |   additive_expression '-' multiplicative_expression { TRACE }
;

multiplicative_expression:
        unary_expression { TRACE }
    |   multiplicative_expression '*' unary_expression { TRACE }
    |   multiplicative_expression '/' unary_expression { TRACE }
    |   multiplicative_expression '%' unary_expression { TRACE }
;

unary_expression:
        T_OP_INC unary_expression                   { TRACE }
    |   T_OP_DEC unary_expression                   { TRACE }
    |   arithmic_unary_operator primary_expression  { TRACE }
    |   logical_unary_expression                    { TRACE }
;

postfix_expression:
        primary_expression      { TRACE }
    |   real_postfix_expression { TRACE }
;

real_postfix_expression:
        postfix_expression T_OP_INC { TRACE }
    |   postfix_expression T_OP_DEC { TRACE }
;

logical_unary_expression:
        postfix_expression                      { TRACE }
    |   logical_unary_operator unary_expression { TRACE }
;

primary_expression:
        qualified_name  { TRACE }
    |   not_just_name   { TRACE }
 ;

arithmic_unary_operator:
        '+' { TRACE }
    |   '-' { TRACE }
;

logical_unary_operator:
        '~' { TRACE }
    |   '!' { TRACE }
;


/* Things that can be used as assignment '=', '+=' etc.. */
assignment_operator:
        '='                { TRACE }
    |   T_PLUS_ASSIGNMENT  { TRACE }
    |   T_MINUS_ASSIGNMENT { TRACE }
    |   T_MUL_ASSIGNMENT   { TRACE }
    |   T_DIV_ASSIGNMENT   { TRACE }
    |   T_MOD_ASSIGNMENT   { TRACE }
    |   T_AND_ASSIGNMENT   { TRACE }
    |   T_OR_ASSIGNMENT    { TRACE }
    |   T_XOR_ASSIGNMENT   { TRACE }
    |   T_SL_ASSIGNMENT    { TRACE }
    |   T_SR_ASSIGNMENT    { TRACE }
;

real_scalar_value:
        T_LNUM   { TRACE }
    |   T_STRING { TRACE }
;


qualified_name:
         T_IDENTIFIER
     |   qualified_name '.' T_IDENTIFIER { TRACE }
;

calling_method_argument_list:
        expression                                     { TRACE }
    |   calling_method_argument_list ',' expression    { TRACE }
    |   /* empty */ { TRACE }
;


not_just_name:
        special_name { TRACE }
    |   complex_primary { TRACE }
;

complex_primary:
        '(' expression ')' { TRACE }
    |   complex_primary_no_parenthesis { TRACE }
;

complex_primary_no_parenthesis:
        T_LNUM          { TRACE }
    |   T_STRING        { TRACE }
    |   T_REGEX         { TRACE }
    |   field_access    { TRACE }
    |   method_call     { TRACE }
    |   data_structure  { TRACE }
;

field_access:
        not_just_name '.' T_IDENTIFIER
    |   qualified_name '.' special_name
;

method_call:
        complex_primary_no_parenthesis '(' calling_method_argument_list ')' { TRACE }
    |   complex_primary_no_parenthesis '(' ')' { TRACE }
    |   qualified_name '(' calling_method_argument_list ')' { TRACE }
    |   qualified_name '(' ')' { TRACE }
;



special_name:
        T_SELF      { TRACE }
    |   T_PARENT    { TRACE }
;


/**
 ************************************************************
 *                  CLASS PARSING
 ************************************************************
 */


/* Statements inside a class: constant and methods */
class_inner_statement_list:
        class_inner_statement                               { TRACE }
    |   class_inner_statement_list class_inner_statement    { TRACE }
;

class_inner_statement:
        constant                    { TRACE }
    |   class_property_definition   { TRACE }
    |   class_method_definition     { TRACE }
;

/* Statements inside an interface: constant and methods */
interface_inner_statement_list:
        interface_inner_statement                                    { TRACE }
    |   interface_inner_statement_list interface_inner_statement     { TRACE }
;

interface_inner_statement:
        interface_method_definition
    |   interface_property_definition
;

interface_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' ';'   { TRACE }
    |                 T_METHOD T_IDENTIFIER '(' method_argument_list ')' ';'   { TRACE }
;

class_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' { TRACE } compound_statement    { TRACE }
    |                 T_METHOD T_IDENTIFIER '(' method_argument_list ')' { TRACE } compound_statement    { TRACE }
    |   interface_method_definition
;

method_argument_list:
        /* empty */
    |   non_empty_method_argument_list
;

non_empty_method_argument_list:
        method_argument                                       { TRACE }
    |   non_empty_method_argument_list ',' method_argument    { TRACE }
;

method_argument:
        T_IDENTIFIER { TRACE }
    |   T_IDENTIFIER '=' primary_expression     { TRACE }
    |   T_IDENTIFIER T_IDENTIFIER { TRACE }
    |   T_IDENTIFIER T_IDENTIFIER '=' primary_expression     { TRACE }
;

constant_list:
        constant                    { TRACE }
    |   constant_list constant      { TRACE }
;

constant:
        T_CONST T_IDENTIFIER '=' real_scalar_value ';' { TRACE }
;

class_definition:
        modifier_list T_CLASS T_IDENTIFIER class_extends class_interface_implements '{' class_inner_statement_list '}' { TRACE }
    |   modifier_list T_CLASS T_IDENTIFIER class_extends class_interface_implements '{' '}' { TRACE }
    |                 T_CLASS T_IDENTIFIER class_extends class_interface_implements '{' class_inner_statement_list '}' { TRACE }
    |                 T_CLASS T_IDENTIFIER class_extends class_interface_implements '{' '}' { TRACE }


;

interface_definition:
        modifier_list T_INTERFACE T_IDENTIFIER class_interface_implements '{' interface_inner_statement_list '}' { TRACE }
    |   modifier_list T_INTERFACE T_IDENTIFIER class_interface_implements '{' '}' { TRACE }
    |                 T_INTERFACE T_IDENTIFIER class_interface_implements '{' interface_inner_statement_list '}' { TRACE }
    |                 T_INTERFACE T_IDENTIFIER class_interface_implements '{' '}' { TRACE }

;


class_property_definition:
        modifier_list T_PROPERTY T_IDENTIFIER '=' expression ';'  { TRACE }
    |   modifier_list T_PROPERTY T_IDENTIFIER ';'                 { TRACE }
;

interface_property_definition:
        modifier_list T_PROPERTY T_IDENTIFIER ';' { TRACE }
;

/* Modifier list can be either empty, or filled */
modifier_list:
       non_empty_modifier_list { TRACE }
;

/* Has at least one modifier */
non_empty_modifier_list:
        modifier { TRACE }
    |   non_empty_modifier_list modifier { TRACE }
;

/* Property and method modifiers. */
modifier:
        T_PROTECTED { TRACE }
    |   T_PUBLIC    { TRACE }
    |   T_PRIVATE   { TRACE }
    |   T_FINAL     { TRACE }
    |   T_ABSTRACT  { TRACE }
    |   T_STATIC    { TRACE }
    |   T_READONLY  { TRACE }
;

/* extends a list of classes, or no extends at all */
class_extends:
        T_EXTENDS class_list { TRACE }
    |   /* empty */
;

/* implements a list of classes, or no implement at all */
class_interface_implements:
        T_IMPLEMENTS class_list { TRACE }
    |   /* empty */
;

/* Comma separated list of classes (for extends and implements) */
class_list:
        class_list ',' T_IDENTIFIER { TRACE }
    |   T_IDENTIFIER { TRACE }
;



/**
 ************************************************************
 * data structure
 ************************************************************
 */

data_structure:
        qualified_name  '[' data_structure_elements     ']' { TRACE }
    |   qualified_name  '[' data_structure_elements ',' ']' { TRACE }
    |   qualified_name  '[' /* empty */                 ']' { TRACE }
    |   complex_primary '[' data_structure_elements     ']' { TRACE }
    |   complex_primary '[' data_structure_elements ',' ']' { TRACE }
    |   complex_primary '[' /* empty */                 ']' { TRACE }
;

data_structure_elements:
        data_structure_element { TRACE }
    |   data_structure_elements ',' data_structure_element { TRACE }
;

data_structure_element:
        assignment_expression { TRACE }
    |   data_structure_element ':' assignment_expression { TRACE }
;

