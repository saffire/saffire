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
%token <sVal> T_VARIABLE
%token <lVal> T_LNUM
%token <sVal> T_STRING

%token T_WHILE T_IF T_PRINT
%nonassoc T_IFX
%nonassoc T_ELSE

%right T_INC T_DEC
%left T_GE T_LE T_EQ T_NE '>' '<' '^'
%left '+' '-'
%left '*' '/'

%type <nPtr> statement expr statement_list

%start saffire

%% /* rules */

saffire:
        function { }
;

function:
        function statement  { saffire_execute($2); saffire_free_node($2); }
    |   /* empty */
;

statement:
        ';'                             { $$ = saffire_opr(';', 2, NULL, NULL); }
    |   expr ';'                        { $$ = $1; }
    |   T_DEC T_VARIABLE                { $$ = saffire_opr(T_DEC, 1, $2); }
    |   T_INC T_VARIABLE                { $$ = saffire_opr(T_INC, 1, $2); }
    |   T_VARIABLE T_DEC                { $$ = saffire_opr(T_DEC, 1, $1); }
    |   T_VARIABLE T_INC                { $$ = saffire_opr(T_INC, 1, $1); }
    |   T_PRINT expr                    { $$ = saffire_opr(T_PRINT, 1, $2); }
    |   T_VARIABLE '=' expr             { $$ = saffire_opr('=', 2, $1, $3); }
    |   T_WHILE '(' expr ')' statement  { $$ = saffire_opr(T_WHILE, 2, $3, $5); }
    |   T_IF '(' expr ')' statement %prec T_IFX
                                        { $$ = saffire_opr(T_IF, 2, $3, $5); }
    |   T_IF '(' expr ')' statement T_ELSE statement
                                        { $$ = saffire_opr(T_IF, 3, $3, $5, $7); }
    |   '{' statement_list '}'          { $$ = $2; }
;

statement_list:
        statement                   { $$ = $1; }
    |   statement_list statement    { $$ = saffire_opr(';', 2, $1, $2); }
;


expr:
        T_LNUM              { $$ = saffire_intCon($1); }
    |   T_STRING            { $$ = saffire_strCon($1); }
    |   T_VARIABLE          { $$ = saffire_var($1); }
    |   expr '+' expr       { $$ = saffire_opr('+', 2, $1, $3); }
    |   expr '-' expr       { $$ = saffire_opr('-', 2, $1, $3); }
    |   expr '*' expr       { $$ = saffire_opr('*', 2, $1, $3); }
    |   expr '/' expr       { $$ = saffire_opr('/', 2, $1, $3); }
    |   expr '<' expr       { $$ = saffire_opr('<', 2, $1, $3); }
    |   expr '>' expr       { $$ = saffire_opr('>', 2, $1, $3); }
    |   expr '^' expr       { $$ = saffire_opr('^', 2, $1, $3); }
    |   expr T_GE expr      { $$ = saffire_opr(T_GE, 2, $1, $3); }
    |   expr T_LE expr      { $$ = saffire_opr(T_LE, 2, $1, $3); }
    |   expr T_NE expr      { $$ = saffire_opr(T_NE, 2, $1, $3); }
    |   expr T_EQ expr      { $$ = saffire_opr(T_EQ, 2, $1, $3); }
    |   '(' expr ')'        { $$ = $2; }

;

