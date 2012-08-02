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
%token <sVal> T_LABEL

%token T_WHILE T_IF T_PRINT T_USE T_AS
%nonassoc T_IFX
%nonassoc T_ELSE

%token T_LIST T_HASH T_LIST_APPEND T_HASH_APPEND

%left T_GE T_LE T_EQ T_NE '>' '<' '^'
%left '+' '-'
%left '*' '/'

%type <nPtr> statement statement_list expr expr_list use_statement
%type <nPtr> hash_vars list_vars hash_scalar_indexes scalar_values
%type <nPtr> assignment_list assignment_list_element

%start saffire

%% /* rules */

saffire:
        /* Use statements are only possible at the top of a file */
        use_statements { }
        function { }
;

use_statements:
        /* Multiple use statements are possible */
        use_statements use_statement { saffire_execute($2); saffire_free_node($2); }
    |   /* empty */
;

use_statement:
        /* use <foo> as <bar>; */
        T_USE T_LABEL T_AS T_LABEL ';' { TRACE $$ = saffire_opr(T_USE, 2, saffire_strCon($2), saffire_strCon($4)); }
        /* use <foo>; */
    |   T_USE T_LABEL ';' { TRACE $$ = saffire_opr(T_USE, 2, saffire_strCon($2), saffire_strCon($2));  }
;

function:
        function statement  { saffire_execute($2); saffire_free_node($2); }
    |   /* empty */
;


statement:
        ';'                             { TRACE $$ = saffire_opr(';', 0); }
    |   expr ';'                        { TRACE $$ = $1; }
    |   assignment_list '=' expr_list   { TRACE $$ = saffire_opr('=', 2, $1, $3); }
/*    |   T_VARIABLE '=' expr             { TRACE $$ = saffire_opr('=', 2, saffire_var($1), $3); } */
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

assignment_list:
        assignment_list ',' assignment_list_element { TRACE $$ = $1; }
    |   assignment_list_element { TRACE $$ = $1; }
;

assignment_list_element:
        T_VARIABLE { TRACE $$ = saffire_var($1); }
;

expr_list:
        expr_list ',' expr { TRACE $$ = $1; }
    |   expr    { TRACE $$ = $1; }
;

expr:
        scalar_values
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
    |   '(' expr ')'        { TRACE $$ = $2; }
;

scalar_values:
        T_LNUM     { TRACE $$ = saffire_intCon($1); }
    |   T_STRING   { TRACE $$ = saffire_strCon($1); }
    |   T_VARIABLE { TRACE $$ = saffire_var($1); }
    |   '[' list_vars ']'   { TRACE $$ = saffire_opr(T_LIST, 1, $2); }
    |   '{' hash_vars '}'   { TRACE $$ = saffire_opr(T_HASH, 1, $2); }
    |   '[' ']'   { TRACE $$ = saffire_opr(T_LIST, 0); }
    |   '{' '}'   { TRACE $$ = saffire_opr(T_HASH, 0); }
;


list_vars:
        /* First item */
        scalar_values { TRACE $$ = saffire_opr(T_LIST_APPEND, 1, $1); }
        /* Middle items */
    |   list_vars ',' scalar_values { TRACE $$ = saffire_opr(T_LIST_APPEND, 2, $1, $3); }
        /* Last item ending with a comma */
    |   list_vars ',' { TRACE $$ = $1 }
;

hash_vars:
        /* First item */
        hash_scalar_indexes ':' scalar_values { TRACE $$ = saffire_opr(T_HASH_APPEND, 2, $1, $3); }
        /* Middle items */
    |   hash_vars ',' hash_scalar_indexes ':' scalar_values { TRACE $$ = saffire_opr(T_HASH_APPEND, 3, $1, $3, $5); }
        /* Last item with a comma */
    |   hash_vars ',' { TRACE $$ = $1 }
;

hash_scalar_indexes:
        /* These can be used as indexes for our hashes */
        T_LNUM     { TRACE $$ = saffire_intCon($1); }
    |   T_STRING   { TRACE $$ = saffire_strCon($1); }
    |   T_VARIABLE { TRACE $$ = saffire_var($1); }
;


