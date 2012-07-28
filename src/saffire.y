%{
    #include <stdio.h>
    #include "node.h"
    #include "saffire_parser.h"

    int yylex(void);
    void yyerror(const char *err) { printf("Error: %s\n", err); }

    #ifdef __DEBUG
        #define YYDEBUG 1
    #else
        #define YYDEBUG 0
    #endif

%}

%union {
    char *sVal;
    long lVal;
    double dVal;
    nodeType *nPtr;
}

%token END 0 "end of file"
%token <sVal> T_LABEL
%token T_PROGRAM
%token T_PRINT
%right T_INC T_DEC
%token T_INC
%token T_DEC
%token <sVal> T_VARIABLE
%token <lVal> T_LNUM
%token <sVal> T_STRING

%type <sVal> inner_statement_list inner_statement expr scalar expr_without_variable variable

%start saffire

%% /* rules */

saffire:
        program_declaration_statement { }
        '{'
        inner_statement_list
        '}' { saffire_do_program_end(); }
;

program_declaration_statement:
        T_PROGRAM T_LABEL { saffire_do_program_begin($2); }
;

inner_statement_list:
        inner_statement_list { }
        inner_statement { $$ = $1; }
    |   /* empty */
;

inner_statement:
        expr { $<sVal>$ = $1; } ';'

expr:
        expr_without_variable { $$ = $1; }
    |   scalar { $$ = $1; }
;

scalar:
        T_STRING { $<sVal>$ = $1; }
    |   T_LNUM { $<lVal>$ = $1; }
;

expr_without_variable:
        variable { $$ = $1; }
    |   variable '=' expr { $$ = $3; saffire_do_assign($1, $3); }
    |   T_DEC variable { $$ = $2; saffire_do_pre_dec($2); }
    |   T_INC variable { $$ = $2; saffire_do_pre_inc($2); }
    |   T_PRINT expr { $$ = $2; saffire_do_print($2); }
    |   variable T_DEC { $$ = $1; saffire_do_post_dec($1); }
    |   variable T_INC { $$ = $1; saffire_do_post_inc($1); }
    |   '(' expr ')' { $$ = $2; }
    |   /* empty */
;

variable:
        T_VARIABLE { $<sVal>$ = $1; }
;


