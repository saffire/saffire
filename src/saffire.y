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

/*
%token use_statement interface_method_definition method_argument_list
%token  class_inner_statements top_statement_list top_statement
%token  class_definition interface_definition constant_list constant
%token  scalar_value interface_inner_statements method_argument real_scalar_value
%token  class_method_definition statement_list statement
%token  calling_method_argument_list interface_property_definition
%token  jump_statement iteration_statement guarding_statement class_property_definition
%token  expression_statement label_statement selection_statement compound_statement expression
%token  assignment_expression assignment_operator unary_expression catch_list catch_header catch
%token  list_element_list hash_element_list hash_scalar_indexes
*/

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
    |   '{' { TRACE("compound statement start") } statement_list '}' { TRACE("compound statement end") }
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
        T_IF expression statement { TRACE("endif") }
    |   T_IF expression statement T_ELSE statement { TRACE("endif-else") }
    |   T_SWITCH expression statement { TRACE("") }
;

iteration_statement:
        T_WHILE expression statement T_ELSE statement { TRACE("endwhile-else") }
    |   T_WHILE expression statement { TRACE("endwhile") }
    |   T_DO statement T_WHILE expression ';' { TRACE("") }
    |   T_FOR '(' expression_statement expression_statement ')' statement { TRACE("") }
    |   T_FOR '(' expression_statement expression_statement expression ')' statement { TRACE("") }
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
        T_TRY compound_statement catch_list                               { TRACE("") }
    |   T_TRY compound_statement catch_list T_FINALLY compound_statement  { TRACE("") }
    |   T_TRY compound_statement            T_FINALLY compound_statement  { TRACE("") }
;

catch_list:
        catch { TRACE("") }
    |   catch_list catch { TRACE("") }
;

catch:
        catch_header compound_statement { TRACE("") }
;

catch_header:
        T_CATCH '(' T_IDENTIFIER T_IDENTIFIER ')' { TRACE("") }
    |   T_CATCH '('              T_IDENTIFIER ')' { TRACE("") }
    |   T_CATCH '('                           ')' { TRACE("") }
;

label_statement:
        T_IDENTIFIER ':' { TRACE("") }
/*    |   T_CASE ConstantExpression ':' */
    |   T_DEFAULT ':' { TRACE("") }
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
    |   equality_expression T_EQ relational_expression { TRACE("eq") }
    |   equality_expression T_NE relational_expression { TRACE("ne") }
    |   equality_expression T_IN relational_expression { TRACE("in") }
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
        primary_expression
    |   arithmic_unary_operator multiplicative_expression { TRACE("") }
    |   logical_unary_operator multiplicative_expression { TRACE("") }
;


arithmic_unary_operator:
        '+' { TRACE("+") }
    |   '-' { TRACE("-") }
;

logical_unary_operator:
        '~' { TRACE("~") }
    |   '!' { TRACE("!") }
;


expression_statement:
        ';' { TRACE("empty expression") }
    |   expression ';' { TRACE("expression") }
;

expression:
        assignment_expression { TRACE("assignment") }
    |   expression ',' assignment_expression { TRACE("assignment") }
;

assignment_expression:
        conditional_expression { TRACE("") }
    |   unary_expression assignment_operator assignment_expression { TRACE("") }
;


/* Things that can be used as assignment '=', '+=' etc.. */
assignment_operator:
        '='                { TRACE("=") }
    |   T_PLUS_ASSIGNMENT  { TRACE("+") }
    |   T_MINUS_ASSIGNMENT { TRACE("-") }
    |   T_MUL_ASSIGNMENT   { TRACE("") }
    |   T_DIV_ASSIGNMENT   { TRACE("") }
    |   T_MOD_ASSIGNMENT   { TRACE("") }
    |   T_AND_ASSIGNMENT   { TRACE("") }
    |   T_OR_ASSIGNMENT    { TRACE("") }
    |   T_XOR_ASSIGNMENT   { TRACE("") }
    |   T_SL_ASSIGNMENT    { TRACE("") }
    |   T_SR_ASSIGNMENT    { TRACE("") }
;

real_scalar_value:
        T_LNUM { TRACE("") }
    |   T_STRING { TRACE("") }
;

scalar_value:
        T_LNUM                      { TRACE("") }
    |   T_STRING                    { TRACE("") }
    |   T_IDENTIFIER                { TRACE("") }
    |   '[' list_element_list ']'   { TRACE("") }
    |   '['                   ']'   { TRACE("") }
    |   '{' hash_element_list '}'   { TRACE("") }
    |   '{'                   '}'   { TRACE("") }
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
    |   calling_method_argument_list ',' expression    { TRACE("") }
    |   /* empty */ { TRACE("") }
;


/**
 ************************************************************
 *                  CLASS PARSING
 ************************************************************
 */


/* Statements inside a class: constant and methods */
class_inner_statements:
        class_inner_statements constant                     { TRACE("is_constant_l") }
    |   constant                                            { TRACE("is_constant") }
    |   class_inner_statements class_property_definition    { TRACE("is_prop_l") }
    |   class_property_definition                           { TRACE("is_prop") }
    |   class_inner_statements class_method_definition      { TRACE("is_meth_l") }
    |   class_method_definition                             { TRACE("is_meth") }
    |   /* empty */ { }
;

/* Statements inside an interface: constant and methods */
interface_inner_statements:
        interface_inner_statements interface_method_definition     { TRACE("") }
    |   interface_method_definition                                { TRACE("") }
    |   interface_inner_statements interface_property_definition   { TRACE("") }
    |   interface_property_definition                              { TRACE("") }
    |   /* empty */ { }
;

interface_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' ';'   { TRACE("") }
;

class_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' { TRACE("Method def") } compound_statement    { TRACE("") }
    |   interface_method_definition
;

method_argument_list:
        method_argument                             { TRACE("") }
    |   method_argument_list ',' method_argument    { TRACE("") }
    |   /* empty */ { }
;

method_argument:
        T_IDENTIFIER                                   { TRACE("") }
    |   T_IDENTIFIER '='  scalar_value                 { TRACE("") }
    |   T_IDENTIFIER T_IDENTIFIER '='  scalar_value    { TRACE("") }
    |   T_IDENTIFIER T_IDENTIFIER                      { TRACE("") }
;

constant_list:
        constant                    { TRACE("") }
    |   constant_list constant      { TRACE("") }
;

constant:
        T_CONST T_IDENTIFIER '=' real_scalar_value ';' { TRACE("") }
;

class_definition:
        modifier_list T_CLASS T_IDENTIFIER class_extends class_interface_implements
        '{'
        class_inner_statements
        '}' { TRACE("") }
;

interface_definition:
        T_INTERFACE T_IDENTIFIER class_interface_implements
        '{'
        interface_inner_statements
        '}' { TRACE("") }
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
    |   non_empty_modifier_list { TRACE("modified_list") }
;

/* Has at least one modifier */
non_empty_modifier_list:
        modifier { TRACE("modified") }
    |   non_empty_modifier_list modifier { TRACE("ne_modifier_list") }
;

/* Property and method modifiers. */
modifier:
        T_PROTECTED { TRACE("protected") }
    |   T_PUBLIC    { TRACE("public") }
    |   T_PRIVATE   { TRACE("private") }
    |   T_FINAL     { TRACE("final") }
    |   T_ABSTRACT  { TRACE("abstract") }
    |   T_STATIC    { TRACE("static") }
    |   T_READONLY  { TRACE("readonly") }
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
        scalar_value { TRACE("") }
        /* Middle items */
    |   list_element_list ',' scalar_value { TRACE("") }
        /* Last item ending with a comma */
    |   list_element_list ',' { TRACE("") }
;

/* Elements inside hashes (key : value pairs) */
hash_element_list:
        /* First item */
        hash_scalar_indexes ':' scalar_value { TRACE("") }
        /* Middle items */
    |   hash_element_list ',' hash_scalar_indexes ':' scalar_value { TRACE("") }
        /* Last item with a comma */
    |   hash_element_list ',' { TRACE("") }
;

/* Hash keys can only be numeric, string or variables */
hash_scalar_indexes:
        /* These can be used as indexes for our hashes */
        T_LNUM       { TRACE("") }
    |   T_STRING     { TRACE("") }
    |   T_IDENTIFIER { TRACE("") }
;


/**
 ************************************************************
 *
 ************************************************************
 */

object_context:
        T_SELF      { TRACE("self") }
    |   T_PARENT    { TRACE("parent") }
;