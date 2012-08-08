%{
    #include <stdio.h>
    #include "node.h"
    #include "saffire_parser.h"

    extern int yylineno;
    int yylex(void);
    void yyerror(const char *err) { printf("Error in line: %d: %s\n", yylineno, err); }

    #ifdef __DEBUG
        #define YYDEBUG 1
        #define TRACE(p) printf("Reduce at line %d: %s\n", __LINE__, p);
    #else
        #define YYDEBUG 0
        #define TRACE(p)
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
%token <sVal> T_STRING
%token <sVal> T_IDENTIFIER

%token T_WHILE T_IF T_USE T_AS T_DO T_SWITCH T_FOR
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

%token T_CATCH T_BREAK T_GOTO T_BREAKELSE T_CONTINUE T_THROW T_RETURN T_FINALLY T_TRY T_DEFAULT T_METHOD

%token T_LIST_START T_LIST_END T_HASH_START T_HASH_END
%token T_OBJECT_OPERATOR T_SELF T_PARENT


%token T_LIST T_HASH T_LIST_APPEND T_HASH_APPEND

%left '=' T_GE T_LE T_EQ T_NE '>' '<' '^' T_IN
%left '+' '-'
%left '*' '/'

%token T_AND T_OR T_SHIFT_LEFT T_SHIFT_RIGHT

%type <nPtr> use_statement interface_method_definition method_argument_list
%type <nPtr> class_inner_statements top_statement_list top_statement
%type <nPtr> class_definition interface_definition constant_list constant
%type <nPtr> scalar_value interface_inner_statements method_argument real_scalar_value
%type <nPtr> class_method_definition statement_list statement
%type <nPtr> calling_method_argument_list interface_property_definition
%type <nPtr> jump_statement iteration_statement guarding_statement class_property_definition
%type <nPtr> expression_statement label_statement selection_statement compound_statement expression
%type <nPtr> assignment_expression assignment_operator unary_expression catch_list catch_header catch
%type <nPtr> list_element_list hash_element_list hash_scalar_indexes

%token T_CLASS T_EXTENDS T_ABSTRACT T_FINAL T_IMPLEMENTS T_INTERFACE
%token T_PUBLIC T_PRIVATE T_PROTECTED T_CONST T_STATIC T_READONLY T_PROPERTY

%start saffire

%% /* rules */


/**
 ************************************************************
 *                  TOP AND USE STATEMENTS
 ************************************************************
 */

saffire:
        /* Use statements are only possible at the top of a file */
        use_statement_list { TRACE("use_statement_list") }

        /* Top statements follow use statements */
        top_statement_list { TRACE("") }
;

use_statement_list:
        non_empty_use_statement_list { TRACE("") }
    |   /* empty */ { TRACE("") }
;

non_empty_use_statement_list:
        use_statement { TRACE("") }
    |   non_empty_use_statement_list use_statement { TRACE("") }
;

use_statement:
        /* use <foo> as <bar>; */
        T_USE T_IDENTIFIER T_AS T_IDENTIFIER ';' { TRACE("") }
        /* use <foo>; */
    |   T_USE T_IDENTIFIER ';' { TRACE("")  }
;


/* Top statements are single (global) statements and/or class/interface/constant */
top_statement_list:
        non_empty_top_statement_list { TRACE("") }
    |   /* empty */ { }
;

non_empty_top_statement_list:
        top_statement{ TRACE("") }
    |   non_empty_top_statement_list top_statement { TRACE("") }
;

/* Top statements can be classes, interfaces, constants, statements */
top_statement:
        class_definition        { TRACE("class definition") }
    |   interface_definition    { TRACE("interface definition") }
    |   constant_list           { TRACE("constant list") }
    |   statement_list          { TRACE("statement list") }
;


/**
 ************************************************************
 *                 BLOCKS & STATEMENTS
 ************************************************************
 */


/* A compound statement is a (set of) statement captured by curly brackets */
compound_statement:
        '{' '}' { TRACE("empty compound") }
    |   '{' statement_list '}' { TRACE("compound with statements") }
;
statement_list:
        statement { TRACE("statement") }
    |   statement_list statement { TRACE("statement element") }
;

statement:
        label_statement             { TRACE("label_statement") }
    |   compound_statement          { TRACE("compound_statement") }
    |   expression_statement        { TRACE("expression_statement") }
    |   selection_statement         { TRACE("selection_statement") }
    |   iteration_statement         { TRACE("iteration_statement") }
    |   jump_statement              { TRACE("jump_statement") }
    |   guarding_statement          { TRACE("guarding_statement") }
;

selection_statement:
        T_IF { TRACE("if") }
            expression
            compound_statement
            { TRACE("endif") }
    |   T_IF { TRACE("if") }
            expression
            compound_statement
        T_ELSE { TRACE("else") }
            compound_statement
            { TRACE("endif") }
    |   T_SWITCH { TRACE("switch") }
            expression
            compound_statement
            { TRACE("") }
;

iteration_statement:
        T_WHILE { TRACE("while(else)") } expression compound_statement T_ELSE { TRACE("elsewhile") } compound_statement { TRACE("endwhile") }
    |   T_WHILE { TRACE("while") } expression compound_statement { TRACE("endwhile") }
    |   T_DO compound_statement T_WHILE expression ';' { TRACE("") }
    |   T_FOR '(' expression_statement expression_statement ')' compound_statement { TRACE("") }
    |   T_FOR '(' expression_statement expression_statement expression ')' compound_statement { TRACE("") }
/*    |    foreach()  { } */
;

jump_statement:
        T_BREAK ';'                 { TRACE("") }
    |   T_BREAKELSE ';'             { TRACE("") }
    |   T_CONTINUE ';'              { TRACE("") }
    |   T_RETURN ';'                { TRACE("") }
    |   T_RETURN expression ';'     { TRACE("") }
    |   T_THROW expression ';'      { TRACE("") }
    |   T_GOTO T_IDENTIFIER ';'     { TRACE("") }
;

guarding_statement:
        T_TRY compound_statement catch_list                               { TRACE("") $$ = saffire_opr(T_TRY, 2, $2, $3); }
    |   T_TRY compound_statement catch_list T_FINALLY compound_statement  { TRACE("") $$ = saffire_opr(T_TRY, 3, $2, $3, $5); }
    |   T_TRY compound_statement            T_FINALLY compound_statement  { TRACE("") $$ = saffire_opr(T_TRY, 2, $2, $4); }
;

catch_list:
        catch { TRACE("") $$ = $1 }
    |   catch_list catch { TRACE("") $$ = saffire_opr(';', 2, $1, $2); }
;

catch:
        catch_header compound_statement { TRACE("") $$ = $2 }
;

catch_header:
        T_CATCH '(' T_IDENTIFIER T_IDENTIFIER ')' { TRACE("") $$ = saffire_strCon($<sVal>3) }
    |   T_CATCH '('              T_IDENTIFIER ')' { TRACE("") $$ = saffire_strCon($<sVal>3) }
    |   T_CATCH '('                           ')' { TRACE("") $$ = saffire_strCon($<sVal>1) }
;

label_statement:
        T_IDENTIFIER ':' { TRACE("") $$ = saffire_strCon($1); }
/*    |   T_CASE ConstantExpression ':' */
    |   T_DEFAULT ':' { TRACE("") $$ = saffire_strCon($<sVal>1); }
;


/**
 ************************************************************
 *                  ASSIGNMENT & EXPRESSION
 ************************************************************
 */

conditional_expression:
        conditional_or_expression { TRACE("") }
    |   conditional_or_expression '?' expression ':' conditional_expression { TRACE("") }
;

conditional_or_expression:
        conditional_and_expression { TRACE("") }
    |   conditional_or_expression T_OR conditional_and_expression { TRACE("") }
;

conditional_and_expression:
        inclusive_or_expression { TRACE("") }
    |   conditional_and_expression T_AND inclusive_or_expression { TRACE("") }
;

inclusive_or_expression:
        exclusive_or_expression { TRACE("") }
    |   inclusive_or_expression '|' exclusive_or_expression { TRACE("") }
;

exclusive_or_expression:
        and_expression { TRACE("") }
    |   exclusive_or_expression '^' and_expression { TRACE("") }
;

and_expression:
        equality_expression { TRACE("") }
    |   and_expression '&' equality_expression { TRACE("") }
;

equality_expression:
        relational_expression { TRACE("") }
    |   equality_expression T_EQ relational_expression { TRACE("") }
    |   equality_expression T_NE relational_expression { TRACE("") }
    |   equality_expression T_IN relational_expression { TRACE("") }
;

relational_expression:
        shift_expression { TRACE("") }
    |   relational_expression '>' shift_expression { TRACE("") }
    |   relational_expression '<' shift_expression { TRACE("") }
    |   relational_expression T_LE shift_expression { TRACE("") }
    |   relational_expression T_GE shift_expression { TRACE("") }
;

shift_expression:
        additive_expression { TRACE("") }
    |   shift_expression T_SHIFT_LEFT additive_expression { TRACE("") }
    |   shift_expression T_SHIFT_RIGHT additive_expression { TRACE("") }
;

additive_expression:
        multiplicative_expression { TRACE("") }
    |   additive_expression '+' multiplicative_expression { TRACE("") }
    |   additive_expression '-' multiplicative_expression { TRACE("") }
;

multiplicative_expression:
        unary_expression { TRACE("") }
    |   multiplicative_expression '*' unary_expression { TRACE("") }
    |   multiplicative_expression '/' unary_expression { TRACE("") }
    |   multiplicative_expression '%' unary_expression { TRACE("") }
;

unary_expression:
        primary_expression { TRACE("") }
    |   unary_operator unary_expression { TRACE("") }
;

expression_statement:
        ';' { TRACE("") }
    |   expression ';' { TRACE("") $$ = $1 }
;

expression:
        assignment_expression { TRACE("") $$ = $1 }
    |   expression ',' assignment_expression { TRACE("") $$ = saffire_opr(';', 2, $1, $3); }
;

assignment_expression:
        conditional_expression { TRACE("") }
    |   unary_expression assignment_operator assignment_expression { TRACE("") }
;


/* Things that can be used as assignment '=', '+=' etc.. */
assignment_operator:
        '='                { TRACE("") $$ = saffire_strCon($<sVal>1); }
    |   T_PLUS_ASSIGNMENT  { TRACE("") $$ = saffire_strCon($<sVal>1); }
    |   T_MINUS_ASSIGNMENT { TRACE("") $$ = saffire_strCon($<sVal>1); }
    |   T_MUL_ASSIGNMENT   { TRACE("") $$ = saffire_strCon($<sVal>1); }
    |   T_DIV_ASSIGNMENT   { TRACE("") $$ = saffire_strCon($<sVal>1); }
    |   T_MOD_ASSIGNMENT   { TRACE("") $$ = saffire_strCon($<sVal>1); }
    |   T_AND_ASSIGNMENT   { TRACE("") $$ = saffire_strCon($<sVal>1); }
    |   T_OR_ASSIGNMENT    { TRACE("") $$ = saffire_strCon($<sVal>1); }
    |   T_XOR_ASSIGNMENT   { TRACE("") $$ = saffire_strCon($<sVal>1); }
    |   T_SL_ASSIGNMENT    { TRACE("") $$ = saffire_strCon($<sVal>1); }
    |   T_SR_ASSIGNMENT    { TRACE("") $$ = saffire_strCon($<sVal>1); }
;

real_scalar_value:
        T_LNUM { TRACE("") $$ = saffire_intCon($1); }
    |   T_STRING { TRACE("") $$ = saffire_strCon($1); }
;

scalar_value:
        T_LNUM                      { TRACE("") $$ = saffire_intCon($1); }
    |   T_STRING                    { TRACE("") $$ = saffire_strCon($1); }
    |   T_IDENTIFIER                { TRACE("") $$ = saffire_strCon($1); }
    |   '[' list_element_list ']'   { TRACE("") $$ = saffire_opr(T_LIST, 1, $2); }
    |   '['                   ']'   { TRACE("") $$ = saffire_opr(T_LIST, 0); }
    |   '{' hash_element_list '}'   { TRACE("") $$ = saffire_opr(T_HASH, 1, $2); }
    |   '{'                   '}'   { TRACE("") $$ = saffire_opr(T_HASH, 0); }
    |   '(' list_element_list ')'   { TRACE("") }
;


primary_expression:
        qualified_name                    { TRACE("") }
    |   object_context '.' qualified_name { TRACE("") }     /* parent.bar(), self.bar() */
    |   T_IDENTIFIER '.' qualified_name   { TRACE("") }     /* Foo.bar(),  foo!.bar() is not possible */
    |   scalar_value                      { TRACE("") }
    |   '(' expression ')'                { TRACE("") }
;

qualified_name:
        T_IDENTIFIER                                      { TRACE("") }
    |   T_IDENTIFIER '(' ')'                              { TRACE("") }
    |   T_IDENTIFIER '(' calling_method_argument_list ')' { TRACE("") }
    |   T_IDENTIFIER     calling_method_argument_list     { TRACE("") }
    |   T_IDENTIFIER '.' qualified_name                   { TRACE("") }
;


calling_method_argument_list:
        expression                                     { TRACE("") }
    |   calling_method_argument_list ',' expression    { TRACE("") $$ = saffire_opr(';', 2, $1, $3); }
    |   /* empty */ { TRACE("") }
;


unary_operator:
        '+' { TRACE("") }
    |   '-' { TRACE("") }
    |   '~' { TRACE("") }
    |   '!' { TRACE("") }
;

/**
 ************************************************************
 *                  CLASS PARSING
 ************************************************************
 */


/* Statements inside a class: constant and methods */
class_inner_statements:
        class_inner_statements constant                     { TRACE("") $$ = saffire_opr(';', 2, $1, $2); }
    |   constant                                            { TRACE("") $$ = $1 }
    |   class_inner_statements class_property_definition    { TRACE("") $$ = saffire_opr(';', 2, $1, $2); }
    |   class_property_definition                           { TRACE("") $$ = $1 }
    |   class_inner_statements class_method_definition      { TRACE("") $$ = saffire_opr(';', 2, $1, $2); }
    |   class_method_definition                             { TRACE("") $$ = $1 }
    |   /* empty */ { }
;

/* Statements inside an interface: constant and methods */
interface_inner_statements:
        interface_inner_statements interface_method_definition     { TRACE("") $$ = saffire_opr(';', 2, $1, $2); }
    |   interface_method_definition                                { TRACE("") $$ = $1 }
    |   interface_inner_statements interface_property_definition   { TRACE("") $$ = saffire_opr(';', 2, $1, $2); }
    |   interface_property_definition                              { TRACE("") $$ = $1 }
    |   /* empty */ { }
;

interface_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' ';'   { TRACE("") }
;

class_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' compound_statement    { TRACE("") $$ = saffire_strCon($3); }
    |   interface_method_definition
;

method_argument_list:
        method_argument                             { TRACE("") $$ = $1; }
    |   method_argument_list ',' method_argument    { TRACE("") $$ = saffire_opr(';', 2, $1, $3); }
    |   /* empty */ { }
;

method_argument:
        T_IDENTIFIER                                   { TRACE("") $$ = saffire_var($1); }
    |   T_IDENTIFIER '='  scalar_value                 { TRACE("") $$ = saffire_var($1); }
    |   T_IDENTIFIER T_IDENTIFIER '='  scalar_value    { TRACE("") $$ = saffire_var($2); }
    |   T_IDENTIFIER T_IDENTIFIER                      { TRACE("") $$ = saffire_var($2); }
;

constant_list:
        constant                    { TRACE("") $$ = $1; }
    |   constant_list constant      { TRACE("") $$ = saffire_opr(';', 2, $1, $2); }
;

constant:
        T_CONST T_IDENTIFIER '=' real_scalar_value ';' { TRACE("") $$ = saffire_opr(';', 2, saffire_var($2), $4); }
;

class_definition:
        modifier_list T_CLASS T_IDENTIFIER class_extends class_interface_implements
        '{'
        class_inner_statements
        '}' { TRACE("") $$ = saffire_strCon($3); }
;

interface_definition:
        T_INTERFACE T_IDENTIFIER class_interface_implements
        '{'
        interface_inner_statements
        '}' { TRACE("") $$ = saffire_strCon($2); }
;


class_property_definition:
        modifier_list T_PROPERTY T_IDENTIFIER '=' scalar_value ';'  { TRACE("") }
    |   modifier_list T_PROPERTY T_IDENTIFIER ';'                   { TRACE("") }
;

interface_property_definition:
        modifier_list T_PROPERTY T_IDENTIFIER ';' { TRACE("") }
;

/* Modifier list can be either empty, or filled */
modifier_list:
        /* empty */ { TRACE("") }
    |   non_empty_modifier_list { TRACE("") }
;

/* Has at least one modifier */
non_empty_modifier_list:
        modifier { TRACE("") }
    |   non_empty_modifier_list modifier { TRACE("") }
;

/* Property and method modifiers. */
modifier:
        T_PROTECTED { TRACE("") }
    |   T_PUBLIC    { TRACE("") }
    |   T_PRIVATE   { TRACE("") }
    |   T_FINAL     { TRACE("") }
    |   T_ABSTRACT  { TRACE("") }
    |   T_STATIC    { TRACE("") }
    |   T_READONLY  { TRACE("") }
;

/* extends a list of classes, or no extends at all */
class_extends:
        T_EXTENDS class_list { TRACE("") }
    |   /* empty */
;

/* implements a list of classes, or no implement at all */
class_interface_implements:
        T_IMPLEMENTS class_list { TRACE("") }
    |   /* empty */
;

/* Comma separated list of classes (for extends and implements) */
class_list:
        class_list ',' T_IDENTIFIER { TRACE("") }
    |   T_IDENTIFIER { TRACE("") }
;


/**
 ************************************************************
 *                  HASHES AND LISTS
 ************************************************************
 */

/* Elements inside lists */
list_element_list:
        /* First item */
        scalar_value { TRACE("") $$ = saffire_opr(T_LIST_APPEND, 1, $1); }
        /* Middle items */
    |   list_element_list ',' scalar_value { TRACE("") $$ = saffire_opr(T_LIST_APPEND, 2, $1, $3); }
        /* Last item ending with a comma */
    |   list_element_list ',' { TRACE("") $$ = $1 }
;

/* Elements inside hashes (key : value pairs) */
hash_element_list:
        /* First item */
        hash_scalar_indexes ':' scalar_value { TRACE("") $$ = saffire_opr(T_HASH_APPEND, 2, $1, $3); }
        /* Middle items */
    |   hash_element_list ',' hash_scalar_indexes ':' scalar_value { TRACE("") $$ = saffire_opr(T_HASH_APPEND, 3, $1, $3, $5); }
        /* Last item with a comma */
    |   hash_element_list ',' { TRACE("") $$ = $1 }
;

/* Hash keys can only be numeric, string or variables */
hash_scalar_indexes:
        /* These can be used as indexes for our hashes */
        T_LNUM       { TRACE("") $$ = saffire_intCon($1); }
    |   T_STRING     { TRACE("") $$ = saffire_strCon($1); }
    |   T_IDENTIFIER { TRACE("") $$ = saffire_var($1); }
;


/**
 ************************************************************
 *
 ************************************************************
 */

object_context:
        T_SELF      { TRACE("") }
    |   T_PARENT    { TRACE("") }
;