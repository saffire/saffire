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

    #define YYPRINT(file, type, value)
%}

%union {
    char *sVal;
    long lVal;
    double dVal;
    t_ast_element *nPtr;
}

%token END 0 "end of file"
%token <lVal> T_LNUM
%token <sVal> T_STRING T_IDENTIFIER T_REGEX

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
%token T_LABEL T_METHOD_CALL T_ARITHMIC T_LOGICAL T_LIST T_PROGRAM T_USE_STATEMENTS

%type <nPtr> program use_statement_list non_empty_use_statement_list use_statement top_statement_list
%type <nPtr> non_empty_top_statement_list top_statement class_definition interface_definition
%type <nPtr> constant_list statement_list compound_statement statement expression_statement jump_statement
%type <nPtr> label_statement selection_statement iteration_statement class_list
%type <nPtr> guarding_statement expression catch_list catch constant_expression data_structure_element data_structure_elements
%type <nPtr> class_interface_implements modifier non_empty_modifier_list class_extends method_argument_list
%type <nPtr> non_empty_method_argument_list interface_inner_statement_list class_inner_statement class_inner_statement_list
%type <nPtr> complex_primary_no_parenthesis not_just_name complex_primary method_call real_postfix_expression
%type <nPtr> postfix_expression unary_expression primary_expression arithmic_unary_operator assignment_operator
%type <nPtr> logical_unary_operator multiplicative_expression additive_expression shift_expression regex_expression
%type <nPtr> relational_expression catch_header conditional_expression assignment_expression modifier_list real_scalar_value
%type <nPtr> constant method_argument interface_inner_statement interface_method_definition interface_property_definition
%type <nPtr> class_method_definition class_property_definition special_name qualified_name calling_method_argument_list
%type <nPtr> data_structure field_access logical_unary_expression equality_expression and_expression inclusive_or_expression
%type <nPtr> conditional_or_expression exclusive_or_expression conditional_and_expression

%type <sVal> '=' T_PLUS_ASSIGNMENT T_MINUS_ASSIGNMENT T_MUL_ASSIGNMENT T_DIV_ASSIGNMENT T_MOD_ASSIGNMENT T_AND_ASSIGNMENT
%type <sVal> T_OR_ASSIGNMENT T_XOR_ASSIGNMENT T_SL_ASSIGNMENT T_SR_ASSIGNMENT '~' '!' '+' '-' T_SELF T_PARENT

%error-verbose

%start saffire

%% /* rules */


/**
 ************************************************************
 *                  TOP AND USE STATEMENTS
 ************************************************************
 */

saffire:
        program { TRACE ast_root = $1; }
;

program:
        use_statement_list top_statement_list { TRACE $$ = saffire_opr(T_PROGRAM,2, $1, $2); }
;

use_statement_list:
        non_empty_use_statement_list { TRACE $$ = $1; }
    |   /* empty */ { TRACE $$ = saffire_nop(); }
;

non_empty_use_statement_list:
        use_statement { TRACE $$ = saffire_opr(T_USE_STATEMENTS, 1, $1); }
    |   non_empty_use_statement_list use_statement { TRACE $$ = saffire_add($$, $2); }
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
    |   /* empty */ { TRACE $$ = saffire_nop(); }
;

non_empty_top_statement_list:
        top_statement{ TRACE $$ = $1; }
    |   non_empty_top_statement_list top_statement { TRACE $$ = saffire_opr(T_LIST, 2, $1, $2); }
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
    |   statement_list statement { TRACE $$ = saffire_opr(T_LIST, 2, $1, $2); }
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
        T_IF expression statement { TRACE $$ = saffire_opr(T_IF, 2, $2, $3); }
    |   T_IF expression statement T_ELSE statement { TRACE $$ = saffire_opr(T_IF, 3, $2, $3, $5); }
    |   T_SWITCH '(' expression ')' compound_statement { TRACE $$ = saffire_opr(T_SWITCH, 2, $3, $5); }
;

iteration_statement:
        T_WHILE expression statement T_ELSE statement { TRACE $$ = saffire_opr(T_WHILE, 3, $2, $3, $5); }
    |   T_WHILE expression statement { TRACE $$ = saffire_opr(T_WHILE, 2, $2, $3); }
    |   T_DO statement T_WHILE expression ';' { TRACE $$ = saffire_opr(T_DO, 2, $2, $4); }
    |   T_FOR '(' expression_statement expression_statement ')' statement { TRACE $$ = saffire_opr(T_FOR, 3, $3, $4, $6); }
    |   T_FOR '(' expression_statement expression_statement expression ')' statement { TRACE $$ = saffire_opr(T_FOR, 4, $3, $4, $5, $7); }
    |   T_FOREACH expression T_AS expression statement { TRACE $$ = saffire_opr(T_FOREACH, 3, $2, $4, $5); }
;

expression_statement:
        ';'             { TRACE $$ = saffire_opr(';', 0); }
    |   expression ';'  { TRACE $$ = $1; }
;

jump_statement:
        T_BREAK ';'                 { TRACE $$ = saffire_opr(T_BREAK, 0); }
    |   T_BREAKELSE ';'             { TRACE $$ = saffire_opr(T_BREAKELSE, 0); }
    |   T_CONTINUE ';'              { TRACE $$ = saffire_opr(T_CONTINUE, 0); }
    |   T_RETURN ';'                { TRACE $$ = saffire_opr(T_RETURN, 0); }
    |   T_RETURN expression ';'     { TRACE $$ = saffire_opr(T_RETURN, 1, $2); }
    |   T_THROW expression ';'      { TRACE $$ = saffire_opr(T_THROW, 1, $2); }
    |   T_GOTO T_IDENTIFIER ';'     { TRACE $$ = saffire_opr(T_GOTO, 1, saffire_strCon($2)); }
;

guarding_statement:
        T_TRY compound_statement catch_list { TRACE $$ = saffire_opr(T_TRY, 2, $2, $3); }
    |   T_TRY compound_statement catch_list T_FINALLY compound_statement  { TRACE $$ = saffire_opr(T_FINALLY, 3, $2, $3, $5); }
    |   T_TRY compound_statement            T_FINALLY compound_statement  { TRACE $$ = saffire_opr(T_FINALLY, 2, $2, $4); }
;

catch_list:
        catch            { TRACE $$ = $1 }
    |   catch_list catch { TRACE $$ = saffire_opr(T_LIST, 2, $1, $2); }
;

catch:
        catch_header compound_statement { TRACE $$ = saffire_opr(T_LIST, 2, $1, $2); }
;

catch_header:
        T_CATCH '(' T_IDENTIFIER T_IDENTIFIER ')' { TRACE $$ = saffire_opr(T_CATCH, 2, saffire_strCon($3), saffire_strCon($4)); }
    |   T_CATCH '('              T_IDENTIFIER ')' { TRACE $$ = saffire_opr(T_CATCH, 1, saffire_strCon($3)); }
    |   T_CATCH '('                           ')' { TRACE $$ = saffire_opr(T_CATCH, 0); }
;

label_statement:
        T_IDENTIFIER ':'                { TRACE $$ = saffire_opr(T_LABEL, 1, saffire_strCon($1)); }
    |   T_CASE constant_expression ':'  { TRACE $$ = saffire_opr(T_CASE, 1, $2);}
    |   T_DEFAULT ':'                   { TRACE $$ = saffire_opr(T_DEFAULT, 0);}
;


/**
 ************************************************************
 *                  ASSIGNMENT & EXPRESSION
 ************************************************************
 */

constant_expression:
        conditional_expression { TRACE $$ = $1; }
;

expression:
        assignment_expression { TRACE $$ = $1; }
    |   expression ',' assignment_expression { TRACE }
;

assignment_expression:
        conditional_expression { TRACE $$ = $1; }
    |   unary_expression assignment_operator assignment_expression { TRACE }
;

conditional_expression:
        conditional_or_expression { TRACE $$ = $1; }
    |   conditional_or_expression '?' expression ':' conditional_expression { TRACE }
;

conditional_or_expression:
        conditional_and_expression { TRACE $$ = $1; }
    |   conditional_or_expression T_OR conditional_and_expression { TRACE }
;

conditional_and_expression:
        inclusive_or_expression { TRACE $$ = $1; }
    |   conditional_and_expression T_AND inclusive_or_expression { TRACE }
;

inclusive_or_expression:
        exclusive_or_expression { TRACE $$ = $1; }
    |   inclusive_or_expression '|' exclusive_or_expression { TRACE }
;

exclusive_or_expression:
        and_expression { TRACE $$ = $1; }
    |   exclusive_or_expression '^' and_expression { TRACE }
;

and_expression:
        equality_expression { TRACE $$ = $1; }
    |   and_expression '&' equality_expression { TRACE }
;

equality_expression:
        relational_expression { TRACE $$ = $1; }
    |   equality_expression T_EQ relational_expression { TRACE $$ = saffire_opr(T_EQ, 2, $1, $3); }
    |   equality_expression T_NE relational_expression { TRACE $$ = saffire_opr(T_NE, 2, $1, $3); }
    |   equality_expression T_IN relational_expression { TRACE $$ = saffire_opr(T_IN, 2, $1, $3); }
;

relational_expression:
        regex_expression { TRACE $$ = $1; }
    |   relational_expression '>' regex_expression { TRACE $$ = saffire_opr('>', 2, $1, $3); }
    |   relational_expression '<' regex_expression { TRACE $$ = saffire_opr('<', 2, $1, $3); }
    |   relational_expression T_LE regex_expression { TRACE $$ = saffire_opr(T_LE, 2, $1, $3); }
    |   relational_expression T_GE regex_expression { TRACE $$ = saffire_opr(T_GE, 2, $1, $3); }
;

regex_expression:
        shift_expression { TRACE $$ = $1; }
    |   regex_expression T_RE T_REGEX { TRACE $$ = saffire_opr(T_RE, 2, $1, $3); }
;

shift_expression:
        additive_expression { TRACE $$ = $1; }
    |   shift_expression T_SHIFT_LEFT additive_expression { TRACE $$ = saffire_opr(T_SHIFT_LEFT, 2, $1, $3); }
    |   shift_expression T_SHIFT_RIGHT additive_expression { TRACE $$ = saffire_opr(T_SHIFT_RIGHT, 2, $1, $3); }
;

additive_expression:
        multiplicative_expression { TRACE $$ = $1; }
    |   additive_expression '+' multiplicative_expression { TRACE $$ = saffire_opr('+', 2, $1, $3); }
    |   additive_expression '-' multiplicative_expression { TRACE $$ = saffire_opr('-', 2, $1, $3); }
;

multiplicative_expression:
        unary_expression { TRACE $$ = $1; }
    |   multiplicative_expression '*' unary_expression { TRACE $$ = saffire_opr('*', 2, $1, $3); }
    |   multiplicative_expression '/' unary_expression { TRACE $$ = saffire_opr('/', 2, $1, $3); }
    |   multiplicative_expression '%' unary_expression { TRACE $$ = saffire_opr('%', 2, $1, $3); }
;

unary_expression:
        T_OP_INC unary_expression                   { TRACE $$ = saffire_opr(T_OP_INC, 1, $2); }
    |   T_OP_DEC unary_expression                   { TRACE $$ = saffire_opr(T_OP_DEC, 1, $2); }
    |   arithmic_unary_operator primary_expression  { TRACE $$ = saffire_opr(T_ARITHMIC, 2, $1, $2); }
    |   logical_unary_expression                    { TRACE $$ = $1; }
;

postfix_expression:
        primary_expression      { TRACE $$ = $1; }
    |   real_postfix_expression { TRACE $$ = $1; }
;

real_postfix_expression:
        postfix_expression T_OP_INC { TRACE $$ = saffire_opr(T_OP_INC, 1, $1); }
    |   postfix_expression T_OP_DEC { TRACE $$ = saffire_opr(T_OP_DEC, 1, $1); }
;

logical_unary_expression:
        postfix_expression                      { TRACE $$ = $1; }
    |   logical_unary_operator unary_expression { TRACE $$ = saffire_opr(T_LOGICAL, 2, $1, $2); }
;

primary_expression:
        qualified_name  { TRACE $$ = $1; }
    |   not_just_name   { TRACE $$ = $1; }
 ;

arithmic_unary_operator:
        '+' { TRACE $$ = saffire_strCon("+"); }
    |   '-' { TRACE $$ = saffire_strCon("-"); }
;

logical_unary_operator:
        '~' { TRACE $$ = saffire_strCon("~"); }
    |   '!' { TRACE $$ = saffire_strCon("!"); }
;


/* Things that can be used as assignment '=', '+=' etc.. */
assignment_operator:
        '='                { TRACE $$ = saffire_strCon("="); }
    |   T_PLUS_ASSIGNMENT  { TRACE $$ = saffire_strCon("+="); }
    |   T_MINUS_ASSIGNMENT { TRACE $$ = saffire_strCon("-="); }
    |   T_MUL_ASSIGNMENT   { TRACE $$ = saffire_strCon("*="); }
    |   T_DIV_ASSIGNMENT   { TRACE $$ = saffire_strCon("/="); }
    |   T_MOD_ASSIGNMENT   { TRACE $$ = saffire_strCon("%="); }
    |   T_AND_ASSIGNMENT   { TRACE $$ = saffire_strCon("&="); }
    |   T_OR_ASSIGNMENT    { TRACE $$ = saffire_strCon("|="); }
    |   T_XOR_ASSIGNMENT   { TRACE $$ = saffire_strCon("^="); }
    |   T_SL_ASSIGNMENT    { TRACE $$ = saffire_strCon("<="); }
    |   T_SR_ASSIGNMENT    { TRACE $$ = saffire_strCon(">="); }
;

real_scalar_value:
        T_LNUM   { TRACE $$ = saffire_intCon($1); }
    |   T_STRING { TRACE $$ = saffire_strCon($1); }
;


qualified_name:
         T_IDENTIFIER { TRACE $$ = saffire_var($1); }
     |   qualified_name '.' T_IDENTIFIER { TRACE $$ = saffire_opr('.', 2, $1, saffire_var($3)); }
;

calling_method_argument_list:
        expression                                     { TRACE $$ = $1; }
    |   calling_method_argument_list ',' expression    { TRACE }
    |   /* empty */ { TRACE }
;


not_just_name:
        special_name { TRACE $$ = $1; }
    |   complex_primary { TRACE $$ = $1; }
;

complex_primary:
        '(' expression ')' { TRACE $$ = $2; }
    |   complex_primary_no_parenthesis { TRACE $$ = $1; }
;

complex_primary_no_parenthesis:
        T_LNUM          { TRACE $$ = saffire_intCon($1); }
    |   T_STRING        { TRACE $$ = saffire_strCon($1); }
    |   T_REGEX         { TRACE $$ = saffire_strCon($1); }
    |   field_access    { TRACE $$ = $1; }
    |   method_call     { TRACE $$ = $1; }
    |   data_structure  { TRACE $$ = $1; }
;

field_access:
        not_just_name '.' T_IDENTIFIER  { TRACE }
    |   qualified_name '.' special_name { TRACE }
;

method_call:
        complex_primary_no_parenthesis '(' calling_method_argument_list ')' { TRACE saffire_opr(T_METHOD_CALL, 2, $1, $3); }
    |   complex_primary_no_parenthesis '(' ')' { TRACE saffire_opr(T_METHOD_CALL, 1, $1); }
    |   qualified_name '(' calling_method_argument_list ')' { TRACE saffire_opr(T_METHOD_CALL, 2, $1, $3); }
    |   qualified_name '(' ')' { TRACE saffire_opr(T_METHOD_CALL, 1, $1); }
;



special_name:
        T_SELF      { TRACE $$ = saffire_strCon("SELF"); }
    |   T_PARENT    { TRACE $$ = saffire_strCon("PARENT"); }
;


/**
 ************************************************************
 *                  CLASS PARSING
 ************************************************************
 */


/* Statements inside a class: constant and methods */
class_inner_statement_list:
        class_inner_statement                               { TRACE $$ = $1; }
    |   class_inner_statement_list class_inner_statement    { TRACE $$ = saffire_opr(T_LIST,2, $1, $2); }
;

class_inner_statement:
        constant                    { TRACE $$ = $1; }
    |   class_property_definition   { TRACE $$ = $1; }
    |   class_method_definition     { TRACE $$ = $1; }
;

/* Statements inside an interface: constant and methods */
interface_inner_statement_list:
        interface_inner_statement                                    { TRACE $$ = $1; }
    |   interface_inner_statement_list interface_inner_statement     { TRACE $$ = saffire_opr(T_LIST,2, $1, $2); }
;

interface_inner_statement:
        interface_method_definition { TRACE $$ = $1; }
    |   interface_property_definition { TRACE $$ = $1; }
;

interface_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' ';'   { TRACE $$ = saffire_var($3); /* @TODO */ }
    |                 T_METHOD T_IDENTIFIER '(' method_argument_list ')' ';'   { TRACE $$ = saffire_var($2); /* @TODO */ }
;

class_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' { TRACE } compound_statement    { TRACE }
    |                 T_METHOD T_IDENTIFIER '(' method_argument_list ')' { TRACE } compound_statement    { TRACE }
    |   interface_method_definition
;

method_argument_list:
        /* empty */  { TRACE }
    |   non_empty_method_argument_list { TRACE $$ = $1; }
;

non_empty_method_argument_list:
        method_argument                                       { TRACE $$ = $1; }
    |   non_empty_method_argument_list ',' method_argument    { TRACE }
;

method_argument:
        T_IDENTIFIER { TRACE $$ = saffire_strCon($1); }
    |   T_IDENTIFIER '=' primary_expression     { TRACE $$ = saffire_var($1); /* @TODO */ }
    |   T_IDENTIFIER T_IDENTIFIER { TRACE $$ = saffire_var($1); /* @TODO */ }
    |   T_IDENTIFIER T_IDENTIFIER '=' primary_expression     { TRACE $$ = saffire_var($1); /* @TODO */ }
;

constant_list:
        constant                    { TRACE $$ = $1; }
    |   constant_list constant      { TRACE $$ = saffire_opr(T_LIST,2, $1, $2); }
;

constant:
        T_CONST T_IDENTIFIER '=' real_scalar_value ';' { TRACE $$ = saffire_opr(T_CONST, 2, saffire_var($2), $4); }
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
       non_empty_modifier_list { TRACE $$ = $1; }
;

/* Has at least one modifier */
non_empty_modifier_list:
        modifier { TRACE $$ = $1; }
    |   non_empty_modifier_list modifier { TRACE $$ = saffire_opr(T_LIST, 2, $1, $2); }
;

/* Property and method modifiers. */
modifier:
        T_PROTECTED { TRACE $$ = saffire_opr(T_PROTECTED, 0); }
    |   T_PUBLIC    { TRACE $$ = saffire_opr(T_PUBLIC, 0); }
    |   T_PRIVATE   { TRACE $$ = saffire_opr(T_PRIVATE, 0); }
    |   T_FINAL     { TRACE $$ = saffire_opr(T_FINAL, 0); }
    |   T_ABSTRACT  { TRACE $$ = saffire_opr(T_ABSTRACT, 0); }
    |   T_STATIC    { TRACE $$ = saffire_opr(T_STATIC, 0); }
    |   T_READONLY  { TRACE $$ = saffire_opr(T_READONLY, 0); }
;

/* extends a list of classes, or no extends at all */
class_extends:
        T_EXTENDS class_list { TRACE $$ = saffire_opr(T_EXTENDS, 1, $2); }
    |   /* empty */  { TRACE }
;

/* implements a list of classes, or no implement at all */
class_interface_implements:
        T_IMPLEMENTS class_list { TRACE $$ = saffire_opr(T_IMPLEMENTS, 1, $2); }
    |   /* empty */  { TRACE }
;

/* Comma separated list of classes (for extends and implements) */
class_list:
        class_list ',' T_IDENTIFIER { TRACE }
    |   T_IDENTIFIER { TRACE $$ = saffire_strCon($1); }
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
        data_structure_element { TRACE $$ = $1; }
    |   data_structure_elements ',' data_structure_element { TRACE }
;

data_structure_element:
        assignment_expression { TRACE $$ = $1; }
    |   data_structure_element ':' assignment_expression { TRACE }
;

%%

   char *get_token_string(int token) {
        for (int i=0; i!=YYNTOKENS; i++) {
            if (token == yytoknum[i]) {
                return (char *)yytname[i];
            }
        }
        return "<unknown>";
    }
