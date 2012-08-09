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

%token T_WHILE T_IF T_USE T_AS T_DO T_SWITCH T_FOR T_FOREACH T_CASE
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

%token T_SELF T_PARENT

%left '=' T_GE T_LE T_EQ T_NE '>' '<' '^' T_IN
%left '+' '-'
%left '*' '/'

%token T_AND T_OR T_SHIFT_LEFT T_SHIFT_RIGHT

/*
%token use_statement interface_method_definition method_argument_list
%token  class_inner_statements top_statement_list top_statement
%token  class_definition interface_definition constant_list constant
%token  scalar_value interface_inner_statements real_scalar_value
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
        top_statement_list { TRACE("top_statement_list") }
;

use_statement_list:
        non_empty_use_statement_list { TRACE("use_staement_list") }
    |   /* empty */ { TRACE("empty statementlist") }
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
        non_empty_top_statement_list { TRACE("ne top statement list") }
    |   /* empty */ { }
;

non_empty_top_statement_list:
        top_statement{ TRACE("top statement") }
    |   non_empty_top_statement_list top_statement { TRACE("top statement next") }
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
    |   T_SWITCH '(' expression ')' compound_statement { TRACE("") }
;

iteration_statement:
        T_WHILE expression statement T_ELSE statement { TRACE("endwhile-else") }
    |   T_WHILE expression statement { TRACE("endwhile") }
    |   T_DO statement T_WHILE expression ';' { TRACE("") }
    |   T_FOR '(' expression_statement expression_statement ')' statement { TRACE("") }
    |   T_FOR '(' expression_statement expression_statement expression ')' statement { TRACE("") }
    |   T_FOREACH expression T_AS expression statement { TRACE("") }
;

expression_statement:
        ';' { TRACE("empty expression") }
    |   expression ';' { TRACE("expression") }
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
    |   T_CASE constant_expression ':' { TRACE("") }
    |   T_DEFAULT ':' { TRACE("") }
;


/**
 ************************************************************
 *                  ASSIGNMENT & EXPRESSION
 ************************************************************
 */

constant_expression:
        conditional_expression { TRACE("cond") }
;

expression:
        assignment_expression { TRACE("expr") }
    |   expression ',' assignment_expression { TRACE("expr ,") }
;

assignment_expression:
        conditional_expression { TRACE("assignment expr") }
    |   unary_expression assignment_operator assignment_expression { TRACE("assignment expr, ") }
;

conditional_expression:
        conditional_or_expression { TRACE("cond expr") }
    |   conditional_or_expression '?' expression ':' conditional_expression { TRACE("cond expr") }
;

conditional_or_expression:
        conditional_and_expression { TRACE("cond or expr") }
    |   conditional_or_expression T_OR conditional_and_expression { TRACE("cond OR") }
;

conditional_and_expression:
        inclusive_or_expression { TRACE("cond and epr") }
    |   conditional_and_expression T_AND inclusive_or_expression { TRACE("cond AND") }
;

inclusive_or_expression:
        exclusive_or_expression { TRACE("incl or expr") }
    |   inclusive_or_expression '|' exclusive_or_expression { TRACE("incl OR") }
;

exclusive_or_expression:
        and_expression { TRACE("excl or expr") }
    |   exclusive_or_expression '^' and_expression { TRACE("excl OR") }
;

and_expression:
        equality_expression { TRACE("and expr") }
    |   and_expression '&' equality_expression { TRACE("AND") }
;

equality_expression:
        relational_expression { TRACE("equal expr") }
    |   equality_expression T_EQ relational_expression { TRACE("eq") }
    |   equality_expression T_NE relational_expression { TRACE("ne") }
    |   equality_expression T_IN relational_expression { TRACE("in") }
;

relational_expression:
        shift_expression { TRACE("relat expr") }
    |   relational_expression '>' shift_expression { TRACE("") }
    |   relational_expression '<' shift_expression { TRACE("") }
    |   relational_expression T_LE shift_expression { TRACE("") }
    |   relational_expression T_GE shift_expression { TRACE("") }
;

shift_expression:
        additive_expression { TRACE("shift expr") }
    |   shift_expression T_SHIFT_LEFT additive_expression { TRACE("") }
    |   shift_expression T_SHIFT_RIGHT additive_expression { TRACE("") }
;

additive_expression:
        multiplicative_expression { TRACE("add expr") }
    |   additive_expression '+' multiplicative_expression { TRACE("") }
    |   additive_expression '-' multiplicative_expression { TRACE("") }
;

multiplicative_expression:
        unary_expression { TRACE("multi expr") }
    |   multiplicative_expression '*' unary_expression { TRACE("me *") }
    |   multiplicative_expression '/' unary_expression { TRACE("me /") }
    |   multiplicative_expression '%' unary_expression { TRACE("me %") }
;

unary_expression:
        arithmic_unary_operator primary_expression { TRACE("unary expr") }
    |   logical_unary_expression
;

logical_unary_expression:
        primary_expression
    |   logical_unary_operator unary_expression
;

primary_expression:
        qualified_name  { TRACE("prim QF") }
    |   not_just_name   { TRACE("notjustname") }
 ;

arithmic_unary_operator:
        '+' { TRACE("+") }
    |   '-' { TRACE("-") }
;

logical_unary_operator:
        '~' { TRACE("~") }
    |   '!' { TRACE("!") }
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


qualified_name:
         T_IDENTIFIER
     |   qualified_name '.' T_IDENTIFIER { TRACE("QF") printf("%s\n", $3); }
;

calling_method_argument_list:
        expression                                     { TRACE("") }
    |   calling_method_argument_list ',' expression    { TRACE("") }
    |   /* empty */ { TRACE("") }
;


not_just_name:
        special_name { TRACE("njn sn") }
    |   complex_primary { TRACE("njn cp") }
;

complex_primary:
        '(' expression ')' { TRACE("cp (expr)") }
    |   complex_primary_no_parenthesis { TRACE("cp cpnp") }
;

complex_primary_no_parenthesis:
        T_LNUM          { TRACE("num") }
    |   T_STRING        { TRACE("string") }
    |   field_access    { TRACE("field xs") }
    |   method_call     { TRACE("methodcall") }
    |   data_structure  { TRACE("datastructure") }
;

field_access:
        not_just_name '.' T_IDENTIFIER
    |   qualified_name '.' special_name
;

method_call:
        complex_primary_no_parenthesis '(' calling_method_argument_list ')' { TRACE("") }
    |   complex_primary_no_parenthesis '(' ')' { TRACE("") }
    |   qualified_name '(' calling_method_argument_list ')' { TRACE("") }
    |   qualified_name '(' ')' { TRACE("") }
;



special_name:
        T_SELF      { TRACE("self") }
    |   T_PARENT    { TRACE("parent") }
;


/**
 ************************************************************
 *                  CLASS PARSING
 ************************************************************
 */


/* Statements inside a class: constant and methods */
class_inner_statement_list:
        class_inner_statement                               { TRACE("class_is") }
    |   class_inner_statement_list class_inner_statement    { TRACE("class_is_l") }
;

class_inner_statement:
        constant                    { TRACE("is_constant") }
    |   class_property_definition   { TRACE("is_property") }
    |   class_method_definition     { TRACE("is_method") }
;

/* Statements inside an interface: constant and methods */
interface_inner_statement_list:
        interface_inner_statement                                    { TRACE("") }
    |   interface_inner_statement_list interface_inner_statement     { TRACE("") }
;

interface_inner_statement:
        interface_method_definition
    |   interface_property_definition
;

interface_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' ';'   { TRACE("") }
    |                 T_METHOD T_IDENTIFIER '(' method_argument_list ')' ';'   { TRACE("") }
;

class_method_definition:
        modifier_list T_METHOD T_IDENTIFIER '(' method_argument_list ')' { TRACE("Method def") } compound_statement    { TRACE("") }
    |                 T_METHOD T_IDENTIFIER '(' method_argument_list ')' { TRACE("Method def") } compound_statement    { TRACE("") }
    |   interface_method_definition
;

method_argument_list:
        /* empty */
    |   non_empty_method_argument_list
;

non_empty_method_argument_list:
        method_argument                                       { TRACE("") }
    |   non_empty_method_argument_list ',' method_argument    { TRACE("") }
;

method_argument:
        T_IDENTIFIER { TRACE("identifier") }
    |   T_IDENTIFIER '=' primary_expression     { TRACE("2x identifier = ") }
    |   T_IDENTIFIER T_IDENTIFIER { TRACE("2x identifier") }
    |   T_IDENTIFIER T_IDENTIFIER '=' primary_expression     { TRACE("2x identifier = ") }
;

constant_list:
        constant                    { TRACE("") }
    |   constant_list constant      { TRACE("") }
;

constant:
        T_CONST T_IDENTIFIER '=' real_scalar_value ';' { TRACE("") }
;

class_definition:
        modifier_list T_CLASS T_IDENTIFIER class_extends class_interface_implements '{' class_inner_statement_list '}' { TRACE("") }
    |   modifier_list T_CLASS T_IDENTIFIER class_extends class_interface_implements '{' '}' { TRACE("") }
    |                 T_CLASS T_IDENTIFIER class_extends class_interface_implements '{' class_inner_statement_list '}' { TRACE("") }
    |                 T_CLASS T_IDENTIFIER class_extends class_interface_implements '{' '}' { TRACE("") }


;

interface_definition:
        modifier_list T_INTERFACE T_IDENTIFIER class_interface_implements '{' interface_inner_statement_list '}' { TRACE("") }
    |   modifier_list T_INTERFACE T_IDENTIFIER class_interface_implements '{' '}' { TRACE("") }
    |                 T_INTERFACE T_IDENTIFIER class_interface_implements '{' interface_inner_statement_list '}' { TRACE("") }
    |                 T_INTERFACE T_IDENTIFIER class_interface_implements '{' '}' { TRACE("") }

;


class_property_definition:
        modifier_list T_PROPERTY T_IDENTIFIER '=' expression ';'  { TRACE("") }
    |   modifier_list T_PROPERTY T_IDENTIFIER ';'                   { TRACE("") }
;

interface_property_definition:
        modifier_list T_PROPERTY T_IDENTIFIER ';' { TRACE("") }
;

/* Modifier list can be either empty, or filled */
modifier_list:
       non_empty_modifier_list { TRACE("modified_list") }
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
 * data structure
 ************************************************************
 */

data_structure:
        qualified_name  '[' data_structure_elements     ']' { TRACE("datastruct[1]") }
    |   qualified_name  '[' data_structure_elements ',' ']' { TRACE("datastruct[2]") }
    |   qualified_name  '[' /* empty */                 ']' { TRACE("datastruct[3]") }
    |   complex_primary '[' data_structure_elements     ']' { TRACE("datastruct[4]") }
    |   complex_primary '[' data_structure_elements ',' ']' { TRACE("datastruct[5]") }
    |   complex_primary '[' /* empty */                 ']' { TRACE("datastruct[6]") }
;

data_structure_elements:
        data_structure_element { TRACE("ds pe") }
    |   data_structure_elements ',' data_structure_element { TRACE("ds pe,") }
;

data_structure_element:
        assignment_expression { TRACE("dse pe") }
    |   data_structure_element ':' assignment_expression { TRACE("dse pe :") }
;

