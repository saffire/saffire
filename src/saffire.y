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
%token <sVal> T_LABEL_METHOD
%token <sVal> T_LABEL
%token <sVal> T_METHOD

%token T_WHILE T_IF T_USE T_AS
%nonassoc T_IFX
%nonassoc T_ELSE

%token T_PLUS_EQUAL
%token T_MINUS_EQUAL
%token T_MUL_EQUAL
%token T_DIV_EQUAL
%token T_MOD_EQUAL
%token T_AND_EQUAL
%token T_OR_EQUAL
%token T_XOR_EQUAL
%token T_SL_EQUAL
%token T_SR_EQUAL

%token T_LIST_START T_LIST_END T_HASH_START T_HASH_END
%token T_OBJECT_OPERATOR


%token T_LIST T_HASH T_LIST_APPEND T_HASH_APPEND

%left '=' T_GE T_LE T_EQ T_NE '>' '<' '^'
%left '+' '-'
%left '*' '/'

%type <nPtr> use_statement interface_method_definition method_argument_list
%type <nPtr> class_inner_statements top_statement_list top_statements
%type <nPtr> class_definition interface_definition constant_list constant
%type <nPtr> scalar_value interface_inner_statements method_argument
%type <nPtr> class_method_definition

%token T_CLASS T_EXTENDS T_ABSTRACT T_FINAL T_IMPLEMENTS T_INTERFACE
%token T_PUBLIC T_PRIVATE T_PROTECTED T_CONST T_STATIC

%start saffire

%% /* rules */

saffire:
        /* Use statements are only possible at the top of a file */
        use_statement_list { }

        /* Top statements follow use statements */
        top_statement_list { }
;

use_statement_list:
        /* Multiple use statements are possible */
        use_statement_list use_statement { saffire_execute($2); saffire_free_node($2); }
    |   /* empty */
;

use_statement:
        /* use <foo> as <bar>; */
        T_USE T_LABEL T_AS T_LABEL ';' { TRACE $$ = saffire_opr(T_USE, 2, saffire_strCon($2), saffire_strCon($4)); }
        /* use <foo>; */
    |   T_USE T_LABEL ';' { TRACE $$ = saffire_opr(T_USE, 2, saffire_strCon($2), saffire_strCon($2));  }
;


top_statement_list:
        top_statement_list top_statements { saffire_execute($2); saffire_free_node($2); }
    |   top_statements { saffire_execute($1); saffire_free_node($1); }
;

/* Top statements can be classes, interfaces, constants, statements */
top_statements:
        class_definition { TRACE $$ = $1 }
    |   interface_definition { TRACE $$ = $1 }
    |   constant_list { TRACE $$ = $1 }
    |   /* Empty */ { }
;


/* Statements inside a class: methods, constants */
class_inner_statements:
        class_inner_statements constant { TRACE $$ = saffire_opr(';', 2, $1, $2); }
    |   class_inner_statements class_method_definition { TRACE $$ = saffire_opr(';', 2, $1, $2); }
    |   constant { TRACE $$ = $1 }
    |   class_method_definition { TRACE $$ = $1 }
    |   /* empty */ { }
;

/* Statements inside an interface: methods, constants */
interface_inner_statements:
        interface_inner_statements constant { TRACE $$ = saffire_opr(';', 2, $1, $2); }
    |   interface_inner_statements interface_method_definition { TRACE $$ = saffire_opr(';', 2, $1, $2); }
    |   constant { TRACE $$ = $1 }
    |   interface_method_definition { TRACE $$ = $1 }
    |   /* empty */ { }
;


interface_method_definition:
        T_PUBLIC T_METHOD T_LABEL_METHOD '(' method_argument_list ')' ';' { TRACE $$ = $3; }
    |   T_PUBLIC T_METHOD T_LABEL '(' method_argument_list ')' ';' { TRACE $$ = $3; }
;

class_method_definition:
        method_keywords T_METHOD T_LABEL_METHOD '(' method_argument_list ')' '{' '}' { TRACE $$ = $3; }
    |   method_keywords T_METHOD T_LABEL        '(' method_argument_list ')' '{' '}' { TRACE $$ = $3; }
    |   T_ABSTRACT method_keywords T_METHOD T_LABEL_METHOD '(' method_argument_list ')' ';' { TRACE $$ = $3; }
    |   T_ABSTRACT method_keywords T_METHOD T_LABEL        '(' method_argument_list ')' ';' { TRACE $$ = $3; }
;

method_argument_list:
        method_argument                             { TRACE $$ = $1; }
    |   method_argument_list ',' method_argument    { TRACE $$ = saffire_opr(';', 2, $1, $3); }
    |   /* empty */ { }
;

method_argument:
        T_VARIABLE                      { TRACE $$ = saffire_var($1); }
    |   T_VARIABLE '='  scalar_value    { TRACE $$ = saffire_var($1); }
;

constant_list:
        constant                    { TRACE $$ = $1; }
    |   constant_list constant      { TRACE $$ = saffire_opr(';', 2, $1, $2); }
;

constant:
        T_CONST T_LABEL '=' scalar_value ';' { TRACE $$ = saffire_opr(';', 2, saffire_var($2), $4); }
;

scalar_value:
        T_LNUM     { TRACE $$ = saffire_intCon($1); }
    |   T_STRING   { TRACE $$ = saffire_strCon($1); }
    |   T_VARIABLE { TRACE $$ = saffire_var($1); }
;

class_definition:
        class_header_keywords T_LABEL class_extends class_interface_implements
        '{'
        class_inner_statements
        '}' { TRACE $$ = saffire_strCon($2); }
;

interface_definition:
        T_INTERFACE T_LABEL class_interface_implements
        '{'
        interface_inner_statements
        '}' { TRACE $$ = saffire_strCon($2); }
;


method_keywords:
        method_mode method_visibility { }
;

method_mode:
        T_FINAL          { }
    |           T_STATIC { }
    |   T_FINAL T_STATIC { }
    |   /* empty */ { }
;

method_visibility:
        T_PROTECTED { }
    |   T_PUBLIC { }
    |   T_PRIVATE { }
;

class_header_keywords:
        T_CLASS { printf("standard class"); }
    |   T_FINAL T_CLASS { printf("Final class"); }
    |   T_ABSTRACT T_CLASS { printf("Abstract class"); }
;

class_extends:
        T_EXTENDS class_list { }
    |   /* empty */

class_interface_implements:
        T_IMPLEMENTS class_list { }
    |   /* empty */

/* Comma separated list of classes (for extends and implements) */
class_list:
        class_list ',' T_LABEL { }
    |   T_LABEL { }
;
