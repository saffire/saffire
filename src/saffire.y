%{
    #include <stdio.h>
    #include "node.h"
    #include "saffire_parser.h"

    extern int yylineno;
    int yylex(void);
    void yyerror(const char *err) { printf("Error in line: %d: %s\n", yylineno, err); }

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
    nodeType *nPtr;
}

%token END 0 "end of file"
%token <lVal> T_LNUM
%token <sVal> T_VARIABLE
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
        ';'                             { TRACE $$ = saffire_opr(';', 2, NULL, NULL); }
    |   expr ';'                        { TRACE $$ = $1; }
    |   T_VARIABLE '=' expr             { TRACE $$ = saffire_opr('=', 2, saffire_var($1), $3); }
    |   T_PRINT expr                    { TRACE $$ = saffire_opr(T_PRINT, 1, $2); }
    |   T_WHILE '(' expr ')' statement  { TRACE $$ = saffire_opr(T_WHILE, 2, $3, $5); }
    |   T_IF '(' expr ')' statement %prec T_IFX
                                        { TRACE $$ = saffire_opr(T_IF, 2, $3, $5); }
    |   T_IF '(' expr ')' statement T_ELSE statement
                                        { TRACE $$ = saffire_opr(T_IF, 3, $3, $5, $7); }
    |   '{' statement_list '}'          { TRACE $$ = $2; }
;

statement_list:
        statement                   { TRACE $$ = $1; }
    |   statement_list statement    { TRACE $$ = saffire_opr(';', 2, $1, $2); }
;


expr:
        T_LNUM              { TRACE $$ = saffire_intCon($1); }
    |   T_STRING            { TRACE $$ = saffire_strCon($1); }
    |   T_VARIABLE          { TRACE $$ = saffire_var($1); }
    |   expr '+' expr       { TRACE $$ = saffire_opr('+', 2, $1, $3); }
    |   expr '-' expr       { TRACE $$ = saffire_opr('-', 2, $1, $3); }
    |   expr '*' expr       { TRACE $$ = saffire_opr('*', 2, $1, $3); }
    |   expr '/' expr       { TRACE $$ = saffire_opr('/', 2, $1, $3); }
    |   expr '<' expr       { TRACE $$ = saffire_opr('<', 2, $1, $3); }
    |   expr '>' expr       { TRACE $$ = saffire_opr('>', 2, $1, $3); }
    |   expr '^' expr       { TRACE $$ = saffire_opr('^', 2, $1, $3); }
    |   expr T_GE expr      { TRACE $$ = saffire_opr(T_GE, 2, $1, $3); }
    |   expr T_LE expr      { TRACE $$ = saffire_opr(T_LE, 2, $1, $3); }
    |   expr T_NE expr      { TRACE $$ = saffire_opr(T_NE, 2, $1, $3); }
    |   expr T_EQ expr      { TRACE $$ = saffire_opr(T_EQ, 2, $1, $3); }
    |   T_DEC T_VARIABLE                { TRACE $$ = saffire_opr(T_DEC, 1, saffire_var($2)  ); }
    |   T_INC T_VARIABLE                { TRACE $$ = saffire_opr(T_INC, 1, saffire_var($2)); }
    |   T_VARIABLE T_DEC                { TRACE $$ = saffire_opr(T_DEC, 1, saffire_var($1)); }
    |   T_VARIABLE T_INC                { TRACE $$ = saffire_opr(T_INC, 1, saffire_var($1)); }
    |   '(' expr ')'        { TRACE $$ = $2; }

;

