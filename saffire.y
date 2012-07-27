%{
    #include <stdio.h>
    #include "saffire_parser.h"

    int yylex(void);
    void yyerror(const char *err) { printf("Error: %s\n", err); }

    #define YYDEBUG 1

%}


%token END 		0 "end of file"
%token T_LABEL
%token T_PROGRAM
%token T_PRINT
%right T_INC T_DEC
%token T_INC
%token T_DEC
%token T_VARIABLE
%token T_LNUM
%token T_TOKEN
%token T_STRING

%start saffire

%% /* rules */

saffire:
        program_declaration_statement { } '{' inner_statement_list '}' { saffire_do_program_end(); }
;

program_declaration_statement:
        T_PROGRAM T_LABEL { saffire_do_program_begin((char *)$2); }
;

inner_statement_list:
        inner_statement_list { } inner_statement { $$ = $1; }
    |   /* empty */
;

inner_statement:
        expr { $$ = $1; } ';'

expr:
        expr_without_variable { $$ = $1; }
    |   scalar { $$ = $1; }
;

scalar:
        T_STRING { $$ = $1; }
    |   T_LNUM { $$ = $1; }
;

expr_without_variable:
        variable { $$ = $1; }
    |   variable '=' expr { $$ = $3; saffire_do_assign((char *)$1, (char *)$3); }
    |   T_DEC variable { $$ = $2; saffire_do_pre_dec((char *)$2); }
    |   T_INC variable { $$ = $2; saffire_do_pre_inc((char *)$2); }
    |   T_PRINT expr { $$ = $2; saffire_do_print((char *)$2); }
    |   variable T_DEC { $$ = $1; saffire_do_post_dec((char *)$1); }
    |   variable T_INC { $$ = $1; saffire_do_post_inc((char *)$1); }
    |   '(' expr ')' { $$ = $2; }
    |   /* empty */
;

variable:
        T_VARIABLE { $$ = $1; }
;


