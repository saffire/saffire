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
%{

/* Define this so we gain external access to our token defines */
#define YY_HEADER_EXPORT_START_CONDITIONS 1

    #include <stdio.h>
    #include "general/output.h"
    #include "general/smm.h"
    #include "general/hashtable.h"
    #include "compiler/parser_helpers.h"
    #include "compiler/ast_nodes.h"
    #include "objects/objects.h"
    #include "debug.h"
    #include "compiler/saffire_parser.h"
    #include "compiler/parser.tab.h"
    #include "compiler/lex.yy.h"

    // Push and pop for parser states
    void saffire_push_state(int state);
    void saffire_pop_state();

    #ifdef __DEBUG
        #define YYDEBUG 1
    #endif

    #ifdef __PARSEDEBUG
        int yydebug = 1;
    #endif

    // Defined so we can access yytoknum, but we don't need to print anything
    #define YYPRINT(yyoutput, char, yyvaluep)

    extern int yylex(union YYSTYPE * yylval, YYLTYPE *yylloc, yyscan_t scanner);

    int yyerror(YYLTYPE *, yyscan_t scanner, SaffireParser *, const char *);

    void yy_exec(SaffireParser *sp);

%}

%define api.pure

%lex-param      { yyscan_t scanner }
%parse-param    { yyscan_t scanner }
%parse-param    { SaffireParser *saffireParser }
%error-verbose

%locations

%union {
    char                 *sVal;     // Holds any string
    long                 lVal;      // Holds any numerical value
    double               dVal;      // Holds any double value
    struct _ast_element  *nPtr;     // Holds any ast-element
}

%token END 0 "end of file"
%token <lVal> T_LNUM "numerical value"
%token <sVal> T_STRING "string" T_IDENTIFIER "identifier" T_REGEX "regular expression"

%type <lVal> modifier modifier_list assignment_operator comparison_operator

/* These must be sorted and used properly */
%token T_WHILE "while" T_IF "if" T_USE "use" T_AS "as" T_DO "do"
%token T_SWITCH "switch" T_FOR "for" T_FOREACH "foreach" T_CASE "case"
%nonassoc T_ELSE
%token T_OP_INC T_OP_DEC T_PLUS_ASSIGNMENT T_MINUS_ASSIGNMENT T_MUL_ASSIGNMENT T_DIV_ASSIGNMENT
%token T_MOD_ASSIGNMENT T_AND_ASSIGNMENT T_OR_ASSIGNMENT T_XOR_ASSIGNMENT T_SL_ASSIGNMENT T_SR_ASSIGNMENT
%token T_CATCH "catch" T_BREAK "break" T_GOTO "goto" T_BREAKELSE "breakelse"
%token T_CONTINUE "continue" T_THROW "throw" T_RETURN "return" T_FINALLY "finally"
%token T_TRY "try" T_DEFAULT "default" T_METHOD "method"
%token T_SELF "self" T_PARENT "parent" T_NS_SEP T_TO
%left T_ASSIGNMENT T_GE T_LE T_EQ T_NE '>' '<' '^' T_IN T_RE T_REGEX
%left '+' '-'
%left '*' '/'
%token T_AND "and" T_OR "or" T_SHIFT_LEFT "shift left" T_SHIFT_RIGHT "shift right" T_COALESCE "coalesce (??)"
%token T_CLASS T_EXTENDS T_ABSTRACT T_FINAL T_IMPLEMENTS T_INHERITS T_INTERFACE
%token T_PUBLIC T_PRIVATE T_PROTECTED T_CONST T_STATIC T_PROPERTY
%token T_LABEL T_CALL T_ARITHMIC T_LOGICAL T_PROGRAM
%token T_FQN T_ARGUMENT_LIST T_ASSIGNMENT T_CALL_ARGUMENT_LIST
%token T_MODIFIERS T_CONSTANTS T_DATA_ELEMENTS T_DATA_STRUCTURE T_DATA_ELEMENT T_METHOD_ARGUMENT
%token T_IMPORT "import" T_FROM "from" T_ELLIPSIS T_SUBSCRIPT T_DATASTRUCT

/* reserved for later use */
%token T_YIELD

%type <nPtr> program use_statement_list non_empty_use_statement_list use_statement top_statement_list
%type <nPtr> non_empty_top_statement_list top_statement class_definition interface_definition
%type <nPtr> statement_list compound_statement statement expression_statement jump_statement
%type <nPtr> label_statement iteration_statement class_list while_statement
%type <nPtr> guarding_statement expression catch_list catch ds_element ds_elements
%type <nPtr> class_implements interface_inherits method_argument_list class_extends
%type <nPtr> non_empty_method_argument_list interface_inner_statement_list class_inner_statement class_inner_statement_list
%type <nPtr> if_statement switch_statement class_constant_definition
%type <nPtr> unary_expression primary_expression pe_no_parenthesis data_structure
%type <nPtr> logical_unary_operator multiplicative_expression additive_expression shift_expression regex_expression
%type <nPtr> catch_header conditional_expression coalesce_expression assignment_expression real_scalar_value
%type <nPtr> method_argument interface_inner_statement interface_method_declaration interface_property_declaration
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

%% /* rules */


/**
 ************************************************************
 *                  TOP AND USE STATEMENTS
 ************************************************************
 */

saffire:
        /* We must convert our ast_root address to an unsigned long */
        program {
            saffireParser->ast = $1;
            saffireParser->eof = 1;
            yy_exec(saffireParser);
        }
;

program:
        /* A program consists of use-statements and a list of top-statements */
        use_statement_list top_statement_list { $$ = ast_node_opr(@1.first_line, T_PROGRAM,2, $1, $2); }
;

use_statement_list:
        non_empty_use_statement_list { $$ = $1; }
    |   /* empty */                  { $$ = ast_node_group(0); }
;

/* A use-statement list with at least one use statement */
non_empty_use_statement_list:
        use_statement { $$ = ast_node_group(1, $1); saffireParser->ast = $1; yy_exec(saffireParser); }
    |   non_empty_use_statement_list use_statement { $$ = ast_node_add($$, $2); saffireParser->ast = $2; yy_exec(saffireParser); }
;

use_statement:
        /* use <foo> as <bar>; */
        T_USE qualified_name T_AS T_IDENTIFIER                        ';' { $$ = ast_node_opr(@1.first_line, T_USE, 2, $2, ast_node_string(@4.first_line, $4));  }
        /* use <foo>; */
    |   T_USE qualified_name                                          ';' { $$ = ast_node_opr(@1.first_line, T_USE, 1, $2); }
        /* import <foo> from <bar> */
    |   T_IMPORT T_IDENTIFIER                   T_FROM qualified_name ';' { $$ = ast_node_opr(@1.first_line, T_IMPORT, 3, ast_node_string(@2.first_line, $2), ast_node_string(@2.first_line, $2), ast_node_string_dup(@4.first_line, $4)); }
        /* import <foo> as <bar> from <baz> */
    |   T_IMPORT T_IDENTIFIER T_AS T_IDENTIFIER T_FROM qualified_name ';' { $$ = ast_node_opr(@1.first_line, T_IMPORT, 3, ast_node_string(@2.first_line, $2), ast_node_string(@4.first_line, $4), ast_node_string_dup(@6.first_line, $6)); }
        /* import <foo> as <bar> */
    |   T_IMPORT T_IDENTIFIER T_AS T_IDENTIFIER                       ';' { $$ = ast_node_opr(@1.first_line, T_IMPORT, 3, ast_node_string(@2.first_line, $2), ast_node_string(@4.first_line, $4), ast_node_string(@2.first_line, $2)); }
        /* import <foo> */
    |   T_IMPORT T_IDENTIFIER                                         ';' { $$ = ast_node_opr(@1.first_line, T_IMPORT, 3, ast_node_string(@2.first_line, $2), ast_node_string(@2.first_line, $2), ast_node_string(@2.first_line, $2)); }
    |   error ';' { yyerrok; }
;

/* Top statements are single (global) statements and/or class/interface/constant */
top_statement_list:
        non_empty_top_statement_list { $$ = $1; }
    |   /* empty */                  { $$ = ast_node_group(0); }
;

/* A top-statement list with at least one top statement */
non_empty_top_statement_list:
        top_statement                              { $$ = ast_node_group(1, $1); saffireParser->ast = $1; yy_exec(saffireParser); }
    |   non_empty_top_statement_list top_statement { $$ = ast_node_add($$, $2); saffireParser->ast = $2; yy_exec(saffireParser); }
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
        statement                { $$ = ast_node_group(1, $1); }
    |   statement_list statement { $$ = ast_node_add($$, $2); }
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
        '{'                '}'  { $$ = ast_node_nop(); }
    |   '{' statement_list '}'  { $$ = $2; }
;

/* if if/else statements */
if_statement:
        T_IF '(' conditional_expression ')' statement                  { $$ = ast_node_opr(@1.first_line, T_IF, 2, $3, $5); }
    |   T_IF '(' conditional_expression ')' statement T_ELSE statement { $$ = ast_node_opr(@1.first_line, T_IF, 3, $3, $5, $7); }
;

/* Switch statement */
switch_statement:
        T_SWITCH '(' conditional_expression ')' { parser_loop_enter(saffireParser, @1.first_line); parser_switch_begin(saffireParser, @1.first_line); } '{' case_statements '}' { parser_loop_leave(saffireParser, @1.first_line); parser_switch_end(saffireParser, @1.first_line);  $$ = ast_node_opr(@1.first_line, T_SWITCH, 2, $3, $7); }
;

case_statements:
        case_statement                 { $$ = ast_node_group(1, $1); }
    |   case_statements case_statement { $$ = ast_node_add($$, $2); }
;

case_statement:
        T_CASE conditional_expression ':'   { parser_switch_case(saffireParser, @1.first_line); }    statement_list { $$ = ast_node_opr(@1.first_line, T_CASE, 2, $2, $5); }
    |   T_CASE conditional_expression ':'   { parser_switch_case(saffireParser, @1.first_line); }    { $$ = ast_node_opr(@1.first_line, T_CASE, 2, $2, ast_node_nop()); }
    |   T_DEFAULT ':'                       { parser_switch_default(saffireParser, @1.first_line); } statement_list { $$ = ast_node_opr(@1.first_line, T_DEFAULT, 1, $4); }
    |   T_DEFAULT ':'                       { parser_switch_default(saffireParser, @1.first_line); } { $$ = ast_node_opr(@1.first_line, T_DEFAULT, 1, ast_node_nop()); }
;


/* while, while else, do/while, for and foreach */
iteration_statement:
        while_statement T_ELSE statement { $$ = ast_node_add($1, $3); }
    |   while_statement                  { $$ = $1; }
    |   T_DO { parser_loop_enter(saffireParser, @1.first_line); } statement T_WHILE '(' conditional_expression ')' ';' { parser_loop_leave(saffireParser, @1.first_line);  $$ = ast_node_opr(@1.first_line, T_DO, 2, $3, $6); }
    |   T_FOR '(' expression_statement expression_statement            ')' { parser_loop_enter(saffireParser, @1.first_line); } statement { $$ = ast_node_opr(@1.first_line, T_FOR, 4, $3, $4, $7, ast_node_nop()); }
    |   T_FOR '(' expression_statement expression_statement expression ')' { parser_loop_enter(saffireParser, @1.first_line); } statement { $$ = ast_node_opr(@1.first_line, T_FOR, 4, $3, $4, $5, $8); }
    |   T_FOREACH '(' expression T_AS ds_element                       ')' { parser_loop_enter(saffireParser, @1.first_line); } statement { parser_loop_leave(saffireParser, @1.first_line);  $$ = ast_node_opr(@1.first_line, T_FOREACH, 2, $3, $5); }
    |   T_FOREACH '(' expression T_AS ds_element ',' T_IDENTIFIER      ')' { parser_loop_enter(saffireParser, @1.first_line); } statement { parser_loop_leave(saffireParser, @1.first_line);  $$ = ast_node_opr(@1.first_line, T_FOREACH, 3, $3, $5, $7); }
;

/* while is separate otherwise we cannot find it's else */
while_statement:
        T_WHILE '(' conditional_expression ')' {
            parser_loop_enter(saffireParser, @1.first_line);
        } statement {
            parser_loop_leave(saffireParser, @1.first_line);
            $$ = ast_node_opr(@1.first_line, T_WHILE, 2, $3, $6);
        }
;

/* An expression is anything that evaluates something */
expression_statement:
        ';'             { $$ = ast_node_nop(); }
    |   expression ';'  { $$ = $1; }
    |   error ';'       { yyerrok; }
;


/* Jumps to another part of the code */
jump_statement:
        T_BREAK ';'                 { parser_validate_break(saffireParser, @1.first_line);  $$ = ast_node_opr(@1.first_line, T_BREAK, 0); }
    |   T_BREAKELSE ';'             { parser_validate_breakelse(saffireParser, @1.first_line);  $$ = ast_node_opr(@1.first_line, T_BREAKELSE, 0); }
    |   T_CONTINUE ';'              { parser_validate_continue(saffireParser, @1.first_line);  $$ = ast_node_opr(@1.first_line, T_CONTINUE, 0); }
    |   T_RETURN ';'                { parser_validate_return(saffireParser, @1.first_line);    $$ = ast_node_opr(@1.first_line, T_RETURN, 1, ast_node_identifier(@1.first_line, "null")); }
    |   T_RETURN expression ';'     { parser_validate_return(saffireParser, @1.first_line);  $$ = ast_node_opr(@1.first_line, T_RETURN, 1, $2); }
    |   T_THROW expression ';'      { $$ = ast_node_opr(@1.first_line, T_THROW, 1, $2); }
    |   T_GOTO T_IDENTIFIER ';'     { $$ = ast_node_opr(@1.first_line, T_GOTO, 1, ast_node_string(@2.first_line, $2)); smm_free($2); }
    |   T_GOTO T_LNUM ';'           { $$ = ast_node_opr(@1.first_line, T_GOTO, 1, ast_node_numerical(@2.first_line, $2)); }                 // @TODO: Should support goto 3; ?
;

/* try/catch try/catch/finally blocks */
guarding_statement:
        T_TRY compound_statement catch_list                               { $$ = ast_node_opr(@1.first_line, T_TRY, 3, $2, $3, ast_node_nop()); }
    |   T_TRY compound_statement catch_list T_FINALLY compound_statement  { $$ = ast_node_opr(@1.first_line, T_TRY, 3, $2, $3, $5); }
;

/* There can be multiple catches on a try{} block */
catch_list:
        catch            { $$ = ast_node_group(1, $1); }
    |   catch_list catch { $$ = ast_node_add($$, $2); }
;

catch:
        catch_header compound_statement { $$ = ast_node_group(2, $1, $2); }
;

catch_header:
        T_CATCH '(' T_IDENTIFIER T_IDENTIFIER ')' { $$ = ast_node_opr(@1.first_line, T_CATCH, 2, ast_node_identifier(@3.first_line, $3), ast_node_identifier(@4.first_line, $4)); smm_free($3); smm_free($4); }
;

label_statement:
        T_IDENTIFIER ':'    { parser_check_label(saffireParser, @1.first_line, $1);  $$ = ast_node_opr(@1.first_line, T_LABEL, 1, ast_node_string(@1.first_line, $1)); smm_free($1); }
    |   T_LNUM ':'          { $$ = ast_node_opr(@1.first_line, T_LABEL, 1, ast_node_numerical(@1.first_line, $1)); }  /* @TODO: should we support goto 4; ? */
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
    |   expression ',' assignment_expression { $$ = ast_node_add($$, $3); }
    |   expression ',' ',' assignment_expression { $$ = ast_node_add($$, ast_node_nop()); $$ = ast_node_add($$, $4); }
;

assignment_expression:
        coalesce_expression { $$ = $1; }
    |   unary_expression assignment_operator assignment_expression { $$ = ast_node_assignment(@1.first_line, $2, $1, $3); }
;

coalesce_expression:
        conditional_expression { $$ = $1; }
    |   conditional_expression T_COALESCE expression { $$ = ast_node_opr(@1.first_line, T_COALESCE, 2, $1, $3); }
;

conditional_expression:
        conditional_or_expression { $$ = $1; }
    |   conditional_or_expression '?' expression ':' conditional_expression { $$ = ast_node_opr(@1.first_line, '?', 3, $1, $3, $5); }
;

conditional_or_expression:
        conditional_and_expression { $$ = $1; }
    |   conditional_or_expression T_OR conditional_and_expression { $$ = ast_node_boolop(@1.first_line, 1, $1, $3);}
;

conditional_and_expression:
        inclusive_or_expression { $$ = $1; }
    |   conditional_and_expression T_AND inclusive_or_expression { $$ = ast_node_boolop(@1.first_line, 0, $1, $3); }
;

inclusive_or_expression:
        exclusive_or_expression { $$ = $1; }
    |   inclusive_or_expression '|' exclusive_or_expression { $$ = ast_node_opr(@1.first_line, '|', 2, $1, $3);}
;

exclusive_or_expression:
        and_expression { $$ = $1; }
    |   exclusive_or_expression '^' and_expression { $$ = ast_node_opr(@1.first_line, '^', 2, $1, $3); }
;

and_expression:
        equality_expression { $$ = $1; }
    |   and_expression '&' equality_expression { $$ = ast_node_opr(@1.first_line, '&', 2, $1, $3); }
;

equality_expression:
        regex_expression { $$ = $1; }
    |   equality_expression comparison_operator regex_expression { $$ = ast_node_comparison(@2.first_line, $2, $1, $3); }
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
    |   regex_expression T_RE T_REGEX { $$ = ast_node_opr(@1.first_line, T_RE, 2, $1, ast_node_string(@3.first_line, $3)); smm_free($3); }
;

shift_expression:
        additive_expression { $$ = $1; }
    |   shift_expression T_SHIFT_LEFT additive_expression { $$ = ast_node_operator(@2.first_line, T_SHIFT_LEFT, $1, $3); }
    |   shift_expression T_SHIFT_RIGHT additive_expression { $$ = ast_node_operator(@2.first_line, T_SHIFT_RIGHT, $1, $3); }
;

additive_expression:
        multiplicative_expression { $$ = $1; }
    |   additive_expression '+' multiplicative_expression { $$ = ast_node_operator(@2.first_line, '+', $1, $3); }
    |   additive_expression '-' multiplicative_expression { $$ = ast_node_operator(@2.first_line, '-', $1, $3); }
;

multiplicative_expression:
        unary_expression { $$ = $1; }
    |   multiplicative_expression '*' unary_expression { $$ = ast_node_operator(@2.first_line, '*', $1, $3); }
    |   multiplicative_expression '/' unary_expression { $$ = ast_node_operator(@2.first_line, '/', $1, $3); }
    |   multiplicative_expression '%' unary_expression { $$ = ast_node_operator(@2.first_line, '%', $1, $3); }
;

unary_expression:
        logical_unary_expression                    { $$ = $1; }
    |   T_OP_INC unary_expression                   { $$ = ast_node_operator(@1.first_line, '+', $2, ast_node_numerical(@1.first_line, 1)); }
    |   T_OP_DEC unary_expression                   { $$ = ast_node_operator(@1.first_line, '-', $2, ast_node_numerical(@1.first_line, 1)); }
;

logical_unary_expression:
        primary_expression                      { $$ = $1; }
    |   logical_unary_operator unary_expression { $$ = ast_node_opr(@1.first_line, T_LOGICAL, 2, $1, $2); }
;

logical_unary_operator:
        '~' { $$ = ast_node_string(@1.first_line, "~"); }
    |   '!' { $$ = ast_node_string(@1.first_line, "!"); }
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
        T_LNUM        { $$ = ast_node_numerical(@1.first_line, $1); }
    |   T_STRING      { $$ = ast_node_string(@1.first_line, $1); smm_free($1); }
    |   T_REGEX       { $$ = ast_node_string(@1.first_line, $1); smm_free($1); }
;

/* Any number, any string, any regex or null|true|false */
scalar_value:
        real_scalar_value   { $$ = $1; }
    |   T_IDENTIFIER        { parser_check_permitted_identifiers(saffireParser, @1.first_line, $1);  $$ = ast_node_identifier(@1.first_line, $1); smm_free($1); }
;

/* This is primary expression */
primary_expression:
        pe_no_parenthesis     { $$ = $1; }
   |    '(' expression ')'    { $$ = $2; }
;

pe_no_parenthesis:
        primary_expression_first_part        { $$ = $1; }
    |   primary_expression '.' T_IDENTIFIER  { $$ = ast_node_property(@1.first_line, $1, ast_node_string(@3.first_line, $3)); }
    |   primary_expression callable          { $$ = ast_node_opr(@1.first_line, T_CALL, 2, $1, ast_node_add($2, ast_node_null())); } /* Add termination varargs list */
    |   primary_expression var_callable      { $$ = ast_node_opr(@1.first_line, T_CALL, 2, $1, $2); }
    |   primary_expression subscription      { $$ = ast_node_opr(@1.first_line, T_SUBSCRIPT, 2, $1, $2); }
    |   primary_expression data_structure    { $$ = ast_node_opr(@1.first_line, T_DATASTRUCT, 2, $1, $2); }
    |   primary_expression T_OP_INC          { $$ = ast_node_operator(@2.first_line, '+', $1, ast_node_numerical(@1.first_line, 1)); }
    |   primary_expression T_OP_DEC          { $$ = ast_node_operator(@2.first_line, '-', $1, ast_node_numerical(@1.first_line, 1)); }
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
    |   qualified_name T_NS_SEP T_IDENTIFIER    { $$ = ast_node_concat($1, "::"); $$ = ast_node_concat($$, $3); }
;

qualified_name_first_part:
        T_IDENTIFIER            { $$ = ast_node_identifier(@1.first_line, $1); }
    |   T_NS_SEP T_IDENTIFIER   { $$ = ast_node_string(@1.first_line, "::"); $$ = ast_node_concat($$, $2); }
;


/* Self and parent are processed differently, since they are separate tokens */
special_name:
        T_SELF    { $$ = ast_node_identifier(@1.first_line, "self"); }
    |   T_PARENT  { $$ = ast_node_identifier(@1.first_line, "parent"); }
;

var_callable:
        '(' calling_method_argument_list ',' T_ELLIPSIS expression ')' { $$ = ast_node_add($2, $5); }
    |   '(' T_ELLIPSIS expression                                  ')' { $$ = ast_node_group(1, $3); }
;

callable:
        '(' calling_method_argument_list ')' { $$ = $2; }
    |   '(' /* empty */                  ')' { $$ = ast_node_group(0); }
;

/* argument list inside a method call*/
calling_method_argument_list:
        assignment_expression                                     { $$ = ast_node_group(1, $1); }
    |   calling_method_argument_list ',' assignment_expression    { $$ = ast_node_add($$, $3); }
;

data_structure:
        '[' ds_elements ']' { $$ = $2; }
;

subscription:
        '[' pe_no_parenthesis T_TO                   ']' { $$ = ast_node_group(2, $2, ast_node_null()); }
    |   '[' pe_no_parenthesis T_TO pe_no_parenthesis ']' { $$ = ast_node_group(2, $2, $4); }
    |   '['                   T_TO pe_no_parenthesis ']' { $$ = ast_node_group(2, ast_node_null(), $3); }
    |   '[' /* empty */                              ']' { $$ = ast_node_group(2, ast_node_null(), ast_node_null()); }
;

/**
 ************************************************************
 *                  CLASS PARSING
 ************************************************************
 */


/* Statements inside a class: constant and methods */
class_inner_statement_list:
        class_inner_statement                               { $$ = ast_node_group(1, $1); }
    |   class_inner_statement_list class_inner_statement    { $$ = ast_node_add($$, $2); }
;

class_inner_statement:
        class_constant_definition   { $$ = $1; }
    |   class_property_definition   { $$ = $1; }
    |   class_method_definition     { $$ = $1; }
;

/* Statements inside an interface: constant and methods */
interface_inner_statement_list:
        interface_inner_statement                                    { $$ = ast_node_group(1, $1); }
    |   interface_inner_statement_list interface_inner_statement     { $$ = ast_node_add($$, $2); }
;

interface_inner_statement:
        interface_method_declaration   { $$ = $1; }
    |   interface_property_declaration { $$ = $1; }
;

interface_method_declaration:
        T_METHOD T_IDENTIFIER '(' method_argument_list ')' ';'   { parser_init_method(saffireParser, @1.first_line, $2); parser_fini_method(saffireParser, @1.first_line);  $$ = ast_node_attribute(@1.first_line, $2, ATTRIB_TYPE_METHOD, 0, ATTRIB_ACCESS_RO, ast_node_nop(), parser_mod_to_methodflags(saffireParser, @1.first_line, 0), $4); smm_free($2); }
;

class_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' {
            parser_init_method(saffireParser, @1.first_line, $3);
            parser_validate_method_modifiers(saffireParser, @1.first_line, $1);
        } compound_statement {
            parser_fini_method(saffireParser, @1.first_line);
            parser_validate_abstract_method_body(saffireParser, @1.first_line, $1, $8);
            $$ = ast_node_attribute(@1.first_line, $3, ATTRIB_TYPE_METHOD, parser_mod_to_visibility(saffireParser, @1.first_line, $1), ATTRIB_ACCESS_RO, $8, parser_mod_to_methodflags(saffireParser, @1.first_line, $1), $5);
            smm_free($3);
        }
    |   modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' ';'   {
            parser_validate_method_modifiers(saffireParser, @1.first_line, $1);
            parser_init_method(saffireParser, @1.first_line, $3);
            parser_fini_method(saffireParser, @1.first_line);
            $$ = ast_node_attribute(@1.first_line, $3, ATTRIB_TYPE_METHOD, parser_mod_to_visibility(saffireParser, @1.first_line, $1), ATTRIB_ACCESS_RO, ast_node_nop(), parser_mod_to_methodflags(saffireParser, @1.first_line, $1), $5);
            smm_free($3);
        }
;

method_argument_list:
        non_empty_method_argument_list                             { $$ = $1; }
    |   /* empty */                                                { $$ = ast_node_nop(); }
    |   non_empty_method_argument_list ',' T_ELLIPSIS T_IDENTIFIER { $$ = $1; ast_node_add($$, ast_node_opr(@1.first_line, T_METHOD_ARGUMENT, 3, ast_node_string(@1.first_line, "..."), ast_node_string(@4.first_line, $4), ast_node_null())); smm_free($4); }
    |   /* empty */                        T_ELLIPSIS T_IDENTIFIER { $$ = ast_node_opr(@1.first_line, T_ARGUMENT_LIST, 0); ast_node_add($$, ast_node_opr(@1.first_line, T_METHOD_ARGUMENT, 3, ast_node_string(@1.first_line, "..."), ast_node_string(@2.first_line, $2), ast_node_null())); smm_free($2); }
;

non_empty_method_argument_list:
        method_argument                                       { $$ = ast_node_opr(@1.first_line, T_ARGUMENT_LIST, 1, $1); }
    |   non_empty_method_argument_list ',' method_argument    { $$ = ast_node_add($$, $3); }
;

method_argument:
        T_IDENTIFIER                                           { $$ = ast_node_opr(@1.first_line, T_METHOD_ARGUMENT, 3, ast_node_null(),     ast_node_string(@1.first_line, $1), ast_node_null()); smm_free($1); }
    |   T_IDENTIFIER T_ASSIGNMENT scalar_value                 { $$ = ast_node_opr(@1.first_line, T_METHOD_ARGUMENT, 3, ast_node_null(),     ast_node_string(@1.first_line, $1), $3        ); smm_free($1); }
    |   T_IDENTIFIER T_IDENTIFIER                              { $$ = ast_node_opr(@1.first_line, T_METHOD_ARGUMENT, 3, ast_node_string(@1.first_line, $1), ast_node_string(@2.first_line, $2), ast_node_null()); smm_free($1); smm_free($2); }
    |   T_IDENTIFIER T_IDENTIFIER T_ASSIGNMENT scalar_value    { $$ = ast_node_opr(@1.first_line, T_METHOD_ARGUMENT, 3, ast_node_string(@1.first_line, $1), ast_node_string(@2.first_line, $2), $4        ); smm_free($1); smm_free($2); }
;

class_definition:
        class_header '{' class_inner_statement_list '}' { $$ = ast_node_class(@4.first_line, saffireParser->parserinfo->active_class, $3); parser_fini_class(saffireParser, @1.first_line); }
    |   class_header '{'                            '}' { $$ = ast_node_class(@3.first_line, saffireParser->parserinfo->active_class, ast_node_nop()); parser_fini_class(saffireParser, @1.first_line); }
;

class_header:
        modifier_list T_CLASS T_IDENTIFIER class_extends class_implements { parser_validate_class_modifiers(saffireParser, @1.first_line, $1); parser_init_class(saffireParser, @1.first_line, parser_mod_to_methodflags(saffireParser, @1.first_line, $1), $3, $4, $5); smm_free($3); }
    |                 T_CLASS T_IDENTIFIER class_extends class_implements { parser_init_class(saffireParser, @1.first_line, 0, $2, $3, $4); smm_free($2); }
;

interface_definition:
        modifier_list T_INTERFACE T_IDENTIFIER interface_inherits '{' interface_inner_statement_list '}' { parser_validate_class_modifiers(saffireParser, @1.first_line, $1); $$ = ast_node_interface(@1.first_line, parser_mod_to_methodflags(saffireParser, @1.first_line, $1), $3, $4, $6); smm_free($3); }
    |   modifier_list T_INTERFACE T_IDENTIFIER interface_inherits '{'                                '}' { parser_validate_class_modifiers(saffireParser, @1.first_line, $1); $$ = ast_node_interface(@1.first_line, parser_mod_to_methodflags(saffireParser, @1.first_line, $1), $3, $4, ast_node_nop()); smm_free($3); }
    |                 T_INTERFACE T_IDENTIFIER interface_inherits '{' interface_inner_statement_list '}' { $$ = ast_node_interface(@1.first_line, 0, $2, $3, $5); smm_free($2); }
    |                 T_INTERFACE T_IDENTIFIER interface_inherits '{'                                '}' { $$ = ast_node_interface(@1.first_line, 0, $2, $3, ast_node_nop()); smm_free($2); };
;

class_property_definition:
        modifier_list T_PROPERTY T_IDENTIFIER T_ASSIGNMENT expression ';'   {
            parser_validate_property_modifiers(saffireParser, @1.first_line, $1);
            $$ = ast_node_attribute(@1.first_line, $3, ATTRIB_TYPE_PROPERTY, parser_mod_to_visibility(saffireParser, @1.first_line, $1), ATTRIB_ACCESS_RW, $5, 0, ast_node_nop());
            smm_free($3);
        }
    |   modifier_list T_PROPERTY T_IDENTIFIER ';' {
        parser_validate_property_modifiers(saffireParser, @1.first_line, $1);
        $$ = ast_node_attribute(@1.first_line, $3, ATTRIB_TYPE_PROPERTY, parser_mod_to_visibility(saffireParser, @1.first_line, $1), ATTRIB_ACCESS_RW, ast_node_null(), 0, ast_node_nop());
        smm_free($3);
    }
;

class_constant_definition:
        modifier_list T_CONST T_IDENTIFIER T_ASSIGNMENT scalar_value ';' { parser_validate_property_modifiers(saffireParser, @1.first_line, $1); $$ = ast_node_attribute(@1.first_line, $3, ATTRIB_TYPE_PROPERTY, parser_mod_to_visibility(saffireParser, @1.first_line, $1), ATTRIB_ACCESS_RO, $5, 0, ast_node_nop()); smm_free($3); }
;


interface_property_declaration:
        modifier_list T_PROPERTY T_IDENTIFIER ';' { parser_validate_property_modifiers(saffireParser, @1.first_line, $1); $$ = ast_node_opr(@1.first_line, T_PROPERTY, 2, parser_mod_to_visibility(saffireParser, @1.first_line, $1), ast_node_identifier(@3.first_line, $3)); smm_free($3); }
;

modifier_list:
        modifier               { $$ = $1; }
    |   modifier_list modifier { parser_validate_flags(saffireParser, @1.first_line, $$, $2); $$ |= $2; }
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
        T_EXTENDS T_IDENTIFIER { $$ = ast_node_string(@2.first_line, $2); smm_free($2); }
    |   /* empty */            { $$ = ast_node_null(); }
;

/* inherits a list of interfaces, or no inherits at all */
interface_inherits:
        T_INHERITS class_list   { $$ = $2; }
    |   /* empty */             { $$ = ast_node_group(0); }
;

/* implements a list of classes, or no implement at all */
class_implements:
        T_IMPLEMENTS class_list { $$ = $2; }
    |   /* empty */             { $$ = ast_node_group(0); }
;


/* Comma separated list of classes (for extends and implements) */
class_list:
        T_IDENTIFIER                { $$ = ast_node_group(1, ast_node_string(@1.first_line, $1)); smm_free($1); }
    |   class_list ',' T_IDENTIFIER { $$ = ast_node_add($$, ast_node_string(@3.first_line, $3)); smm_free($3); }
;



/**
 ************************************************************
 * data structures
 ************************************************************
 */

ds_elements:
        ds_element                 { $$ = ast_node_group(1, $1); }
    |   ds_elements ',' ds_element { $$ = ast_node_add($$, $3); }
;

ds_element:
        assignment_expression                { $$ = ast_node_group(1, $1); }
    |   ds_element ':' assignment_expression { $$ = ast_node_add($$, $3); }
;

%%

// Use our own saffire memory manager
void *yyalloc (size_t bytes, yyscan_t scanner) {
    return smm_malloc(bytes);
}
void *yyrealloc (void *ptr, size_t bytes, yyscan_t scanner) {
    return smm_realloc(ptr, bytes);
}
void yyfree (void *ptr, yyscan_t scanner) {
    return smm_free(ptr);
}


extern int flush_buffer(yyscan_t scanner);

/**
 * Displays error based on location, scanner and interpreter structure. Will continue or fail depending on interpreter mode
 */
int yyerror(YYLTYPE *yylloc, yyscan_t scanner, SaffireParser *sp, const char *message) {
    char buf[2048];

    snprintf(buf, 2047, "%s, found in %s on line %d\n", message, sp->filename, yylloc->first_line);
    warning(buf);

    // Flush current buffer, and return when we are in interactive/REPL mode.
    if (sp->mode == SAFFIRE_EXECMODE_REPL) {
        flush_buffer(scanner);
        return 0;
    }

    // Otherwise, exit.
    exit(1);
}


/**
 * Returns the actual name of a token, must be implemented here, because we have no access
 * to the yytoknum and yytname otherwise in other source files.
 */
char *get_token_string(int token) {
    for (int i=0; i!=YYNTOKENS; i++) {
        if (token == yytoknum[i]) {
            return (char *)yytname[i];
        }
    }
    return "<unknown>";
}


/**
 * Hook that will be called on each finalized (top/use) statement. Useful to handle
 * interactive sessions like the repl.
 */
void yy_exec(SaffireParser *sp) {
    // Check if we need to handle the instructions (hook for mostly the repl)
    if (! sp->yyexec) {
        return;
    }

    // No AST to run
    if (! sp->ast) {
        return;
    }

    sp->yyexec(sp);
    sp->ast = NULL;
}
