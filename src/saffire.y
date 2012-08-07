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
%token <sVal> T_IDENTIFIER_METHOD
%token <sVal> T_IDENTIFIER
%token <sVal> T_METHOD

%token T_WHILE T_IF T_USE T_AS
%nonassoc T_IFX
%nonassoc T_ELSE

%token T_PLUS_ASSIGNMENT
%token T_MINUS_ASSIGNMENT
%token T_MUL_ASSIGNMENT
%token T_DIV_ASSIGNMENT
%token T_MOD_ASSIGNMENT
%token T_AND_ASSIGNMENT
%token T_OR_ASSIGNMENT
%token T_XOR_ASSIGNMENT
%token T_SL_ASSIGNMENT
%token T_SR_ASSIGNMENT

%token T_CATCH T_BREAK T_GOTO T_BREAKELSE T_CONTINUE T_THROW T_RETURN T_FINALLY T_TRY T_DEFAULT

%token T_LIST_START T_LIST_END T_HASH_START T_HASH_END
%token T_OBJECT_OPERATOR


%token T_LIST T_HASH T_LIST_APPEND T_HASH_APPEND

%left '=' T_GE T_LE T_EQ T_NE '>' '<' '^'
%left '+' '-'
%left '*' '/'

%token T_AND T_OR T_SHIFT_LEFT T_SHIFT_RIGHT

%type <nPtr> use_statement interface_method_definition method_argument_list
%type <nPtr> class_inner_statements top_statement_list top_statement
%type <nPtr> class_definition interface_definition constant_list constant
%type <nPtr> scalar_value interface_inner_statements method_argument
%type <nPtr> class_method_definition method_visibility statement_list statement
%type <nPtr> constant_definition calling_method_argument_list
%type <nPtr> jump_statement iteration_statement guarding_statement
%type <nPtr> expression_statement label_statement selection_statement block expression
%type <nPtr> assignment_expression assignment_operator unary_expression catch_list catch_header catch
%type <nPtr> list_var_list hash_var_list hash_scalar_indexes

%token T_CLASS T_EXTENDS T_ABSTRACT T_FINAL T_IMPLEMENTS T_INTERFACE
%token T_PUBLIC T_PRIVATE T_PROTECTED T_CONST T_STATIC

%start saffire

%% /* rules */


/**
 ************************************************************
 *                  TOP AND USE STATEMENTS
 ************************************************************
 */

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
        T_USE T_IDENTIFIER T_AS T_IDENTIFIER ';' { TRACE $$ = saffire_opr(T_USE, 2, saffire_strCon($2), saffire_strCon($4)); }
        /* use <foo>; */
    |   T_USE T_IDENTIFIER ';' { TRACE $$ = saffire_opr(T_USE, 2, saffire_strCon($2), saffire_strCon($2));  }
;


/* Top statements are single (global) statements and/or class/interface/constant */
top_statement_list:
        top_statement_list top_statement { saffire_execute($2); saffire_free_node($2); }
    |   top_statement { saffire_execute($1); saffire_free_node($1); }
;

/* Top statements can be classes, interfaces, constants, statements */
top_statement:
        class_definition        { TRACE $$ = $1 }
    |   interface_definition    { TRACE $$ = $1 }
    |   constant_list           { TRACE $$ = $1 }
    |   statement_list          { TRACE $$ = $1 }
;


/**
 ************************************************************
 *                 BLOCKS & STATEMENTS
 ************************************************************
 */


/*  */
statement_list:
        statement_list statement {  }
    |   statement {  }
;

/* A block is a (set of) statement captured by curly brackets */
block:
        '{' statement '}'           { TRACE $$ = $2 }
    |   '{' '}'                     { }
;

statement:
        label_statement             { TRACE $$ = $1 }
    |   expression_statement        { TRACE $$ = $1 }
    |   selection_statement         { TRACE $$ = $1 }
    |   iteration_statement         { TRACE $$ = $1 }
    |   jump_statement              { TRACE $$ = $1 }
    |   guarding_statement          { TRACE $$ = $1 }
    |   block                       { TRACE $$ = $1 }
;

selection_statement:
        /*  if */       { }
    |   /*  if else */  { }
    |   /*  switch */   { }
;

iteration_statement:
        /* while */     { }
    |   /* do */        { }
    |   /* for() */     { }
    |   /* foreach() */ { }
;

jump_statement:
        T_BREAK ';'                 { }
    |   T_BREAKELSE ';'             { }
    |   T_CONTINUE ';'              { }
    |   T_RETURN ';'                { }
    |   T_RETURN expression ';'     { }
    |   T_THROW expression ';'      { }
    |   T_GOTO T_IDENTIFIER ';'     { }
;

guarding_statement:
        T_TRY block catch_list                  { TRACE $$ = saffire_opr(T_TRY, 2, $2, $3); }
    |   T_TRY block catch_list T_FINALLY block  { TRACE $$ = saffire_opr(T_TRY, 3, $2, $3, $5); }
    |   T_TRY block            T_FINALLY block  { TRACE $$ = saffire_opr(T_TRY, 2, $2, $4); }
;

catch_list:
        catch { TRACE $$ = $1 }
    |   catch_list catch { TRACE $$ = saffire_opr(';', 2, $1, $2); }
;

catch:
    catch_header block { TRACE $$ = $2 }
;

catch_header:
        T_CATCH '(' T_IDENTIFIER T_VARIABLE ')' { TRACE $$ = saffire_strCon($<sVal>3) }
    |   T_CATCH '('              T_VARIABLE ')' { TRACE $$ = saffire_strCon($<sVal>3) }
    |   T_CATCH '('                         ')' { TRACE $$ = saffire_strCon($<sVal>1) }
;

label_statement:
        T_IDENTIFIER ':' { TRACE $$ = saffire_strCon($1); }
/*    |   T_CASE ConstantExpression ':' */
    |   T_DEFAULT ':' { TRACE $$ = saffire_strCon($<sVal>1); }
;


/**
 ************************************************************
 *                  ASSIGNMENT & EXPRESSION
 ************************************************************
 */

constant_expression:
        conditional_expression { TRACE }
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
;

relational_expression:
        shift_expression { TRACE }
    |   relational_expression '>' shift_expression { TRACE }
    |   relational_expression '<' shift_expression { TRACE }
    |   relational_expression T_LE shift_expression { TRACE }
    |   relational_expression T_GE shift_expression { TRACE }
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
        primary_expression { TRACE }
;

primary_expression:
        T_VARIABLE { TRACE }
    |   T_STRING { TRACE }
;

expression_statement:
        ';' { TRACE }
    |   expression ';' { TRACE $$ = $1 }
;

expression:
        assignment_expression { TRACE $$ = $1 }
    |   expression ',' assignment_expression { TRACE $$ = saffire_opr(';', 2, $1, $3); }
;

assignment_expression:
        conditional_expression { TRACE }
    |   unary_expression assignment_operator assignment_expression { TRACE }
;


/* Things that can be used as assignment '=', '+=' etc.. */
assignment_operator:
        '='                { TRACE $$ = saffire_strCon($<sVal>1); }
    |   T_PLUS_ASSIGNMENT  { TRACE $$ = saffire_strCon($<sVal>1); }
    |   T_MINUS_ASSIGNMENT { TRACE $$ = saffire_strCon($<sVal>1); }
    |   T_MUL_ASSIGNMENT   { TRACE $$ = saffire_strCon($<sVal>1); }
    |   T_DIV_ASSIGNMENT   { TRACE $$ = saffire_strCon($<sVal>1); }
    |   T_MOD_ASSIGNMENT   { TRACE $$ = saffire_strCon($<sVal>1); }
    |   T_AND_ASSIGNMENT   { TRACE $$ = saffire_strCon($<sVal>1); }
    |   T_OR_ASSIGNMENT    { TRACE $$ = saffire_strCon($<sVal>1); }
    |   T_XOR_ASSIGNMENT   { TRACE $$ = saffire_strCon($<sVal>1); }
    |   T_SL_ASSIGNMENT    { TRACE $$ = saffire_strCon($<sVal>1); }
    |   T_SR_ASSIGNMENT    { TRACE $$ = saffire_strCon($<sVal>1); }
;

scalar_value:
        T_LNUM                  { TRACE $$ = saffire_intCon($1); }
    |   T_STRING                { TRACE $$ = saffire_strCon($1); }
    |   T_VARIABLE              { TRACE $$ = saffire_var($1); }
    |   T_IDENTIFIER            { TRACE $$ = saffire_strCon($1); }
    |   '[' list_var_list ']'   { TRACE $$ = saffire_opr(T_LIST, 1, $2); }
    |   '['               ']'   { TRACE $$ = saffire_opr(T_LIST, 0); }
    |   '{' hash_var_list '}'   { TRACE $$ = saffire_opr(T_HASH, 1, $2); }
    |   '{'               '}'   { TRACE $$ = saffire_opr(T_HASH, 0); }
;


primary_expression:
        qualified_name                    { TRACE }
    |   T_IDENTIFIER '.' qualified_name   { TRACE }
    |   T_VARIABLE '.' qualified_name     { TRACE }
    |   scalar_value                      { TRACE }
    |   '(' expression ')'                { TRACE }
;

qualified_name:
        method_or_var                                      { TRACE }
    |   method_or_var '(' ')'                              { TRACE }
    |   method_or_var '(' calling_method_argument_list ')' { TRACE }
    |   method_or_var     calling_method_argument_list     { TRACE }
    |   method_or_var '.' qualified_name                   { TRACE }
;

method_or_var:
        T_IDENTIFIER        { TRACE }
    |   T_IDENTIFIER_METHOD { TRACE }
;


calling_method_argument_list:
        expression                                     { TRACE }
    |   scalar_value                                   { TRACE }
    |   calling_method_argument_list ',' expression    { TRACE $$ = saffire_opr(';', 2, $1, $3); }
    |   calling_method_argument_list ',' scalar_value  { TRACE $$ = saffire_opr(';', 2, $1, $3); }
    |   /* empty */ { }
;

/**
 ************************************************************
 *                  CLASS PARSING
 ************************************************************
 */


/* Statements inside a class: constant and methods */
class_inner_statements:
        class_inner_statements constant                { TRACE $$ = saffire_opr(';', 2, $1, $2); }
    |   class_inner_statements class_method_definition { TRACE $$ = saffire_opr(';', 2, $1, $2); }
    |   constant                                       { TRACE $$ = $1 }
    |   class_method_definition                        { TRACE $$ = $1 }
    |   /* empty */ { }
;

/* Statements inside an interface: constant and methods */
interface_inner_statements:
        interface_inner_statements constant_definition          { TRACE $$ = saffire_opr(';', 2, $1, $2); }
    |   interface_inner_statements interface_method_definition  { TRACE $$ = saffire_opr(';', 2, $1, $2); }
    |   constant_definition                                     { TRACE $$ = $1 }
    |   interface_method_definition                             { TRACE $$ = $1 }
    |   /* empty */ { }
;

interface_method_definition:
        T_PUBLIC T_METHOD method_or_var '(' method_argument_list ')' ';'   { TRACE }
;

class_method_definition:
                   method_keywords T_METHOD method_or_var '(' method_argument_list ')' block    { TRACE $$ = saffire_strCon($2); }
    |   T_ABSTRACT method_keywords T_METHOD method_or_var '(' method_argument_list ')' ';'      { TRACE $$ = saffire_strCon($3); }
;

method_argument_list:
        method_argument                             { TRACE $$ = $1; }
    |   method_argument_list ',' method_argument    { TRACE $$ = saffire_opr(';', 2, $1, $3); }
    |   /* empty */ { }
;

method_argument:
        T_VARIABLE                                   { TRACE $$ = saffire_var($1); }
    |   T_VARIABLE '='  scalar_value                 { TRACE $$ = saffire_var($1); }
    |   T_IDENTIFIER T_VARIABLE '='  scalar_value    { TRACE $$ = saffire_var($2); }
    |   T_IDENTIFIER T_VARIABLE                      { TRACE $$ = saffire_var($2); }
;

constant_list:
        constant                    { TRACE $$ = $1; }
    |   constant_list constant      { TRACE $$ = saffire_opr(';', 2, $1, $2); }
;

constant:
        T_CONST T_IDENTIFIER '=' scalar_value ';' { TRACE $$ = saffire_opr(';', 2, saffire_var($2), $4); }
;

constant_definition:
        T_CONST T_IDENTIFIER ';' { TRACE $$ = saffire_var($2); }
;

class_definition:
        class_header_keywords T_IDENTIFIER class_extends class_interface_implements
        '{'
        class_inner_statements
        '}' { TRACE $$ = saffire_strCon($2); }
;

interface_definition:
        T_INTERFACE T_IDENTIFIER class_interface_implements
        '{'
        interface_inner_statements
        '}' { TRACE $$ = saffire_strCon($2); }
;


method_keywords:
        method_mode method_visibility { }
;

method_mode:
        T_FINAL          { TRACE }
    |           T_STATIC { TRACE }
    |   T_FINAL T_STATIC { TRACE }
    |   /* empty */ { }
;

/* methods MUST be one of these. There is no default visibility */
method_visibility:
        T_PROTECTED { TRACE }
    |   T_PUBLIC    { TRACE }
    |   T_PRIVATE   { TRACE }
;

/* class keywords. Final, abstract or none */
class_header_keywords:
                   T_CLASS { TRACE }
    |   T_FINAL    T_CLASS { TRACE }
    |   T_ABSTRACT T_CLASS { TRACE }
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
 *                  HASHES AND LISTS
 ************************************************************
 */

list_var_list:
        /* First item */
        scalar_value { TRACE $$ = saffire_opr(T_LIST_APPEND, 1, $1); }
        /* Middle items */
    |   list_var_list ',' scalar_value { TRACE $$ = saffire_opr(T_LIST_APPEND, 2, $1, $3); }
        /* Last item ending with a comma */
    |   list_var_list ',' { TRACE $$ = $1 }
;

hash_var_list:
        /* First item */
        hash_scalar_indexes ':' scalar_value { TRACE $$ = saffire_opr(T_HASH_APPEND, 2, $1, $3); }
        /* Middle items */
    |   hash_var_list ',' hash_scalar_indexes ':' scalar_value { TRACE $$ = saffire_opr(T_HASH_APPEND, 3, $1, $3, $5); }
        /* Last item with a comma */
    |   hash_var_list ',' { TRACE $$ = $1 }
;

hash_scalar_indexes:
        /* These can be used as indexes for our hashes */
        T_LNUM     { TRACE $$ = saffire_intCon($1); }
    |   T_STRING   { TRACE $$ = saffire_strCon($1); }
    |   T_VARIABLE { TRACE $$ = saffire_var($1); }
;

