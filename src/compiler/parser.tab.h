
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     END = 0,
     T_LNUM = 258,
     T_STRING = 259,
     T_IDENTIFIER = 260,
     T_REGEX = 261,
     T_WHILE = 262,
     T_IF = 263,
     T_USE = 264,
     T_AS = 265,
     T_DO = 266,
     T_SWITCH = 267,
     T_FOR = 268,
     T_FOREACH = 269,
     T_CASE = 270,
     T_ELSE = 271,
     T_OP_INC = 272,
     T_OP_DEC = 273,
     T_PLUS_ASSIGNMENT = 274,
     T_MINUS_ASSIGNMENT = 275,
     T_MUL_ASSIGNMENT = 276,
     T_DIV_ASSIGNMENT = 277,
     T_MOD_ASSIGNMENT = 278,
     T_AND_ASSIGNMENT = 279,
     T_OR_ASSIGNMENT = 280,
     T_XOR_ASSIGNMENT = 281,
     T_SL_ASSIGNMENT = 282,
     T_SR_ASSIGNMENT = 283,
     T_CATCH = 284,
     T_BREAK = 285,
     T_GOTO = 286,
     T_BREAKELSE = 287,
     T_CONTINUE = 288,
     T_THROW = 289,
     T_RETURN = 290,
     T_FINALLY = 291,
     T_TRY = 292,
     T_DEFAULT = 293,
     T_METHOD = 294,
     T_SELF = 295,
     T_PARENT = 296,
     T_RE = 297,
     T_IN = 298,
     T_NE = 299,
     T_EQ = 300,
     T_LE = 301,
     T_GE = 302,
     T_AND = 303,
     T_OR = 304,
     T_SHIFT_LEFT = 305,
     T_SHIFT_RIGHT = 306,
     T_CLASS = 307,
     T_EXTENDS = 308,
     T_ABSTRACT = 309,
     T_FINAL = 310,
     T_IMPLEMENTS = 311,
     T_INTERFACE = 312,
     T_PUBLIC = 313,
     T_PRIVATE = 314,
     T_PROTECTED = 315,
     T_CONST = 316,
     T_STATIC = 317,
     T_READONLY = 318,
     T_PROPERTY = 319,
     T_LABEL = 320,
     T_METHOD_CALL = 321,
     T_ARITHMIC = 322,
     T_LOGICAL = 323,
     T_TOP_STATEMENTS = 324,
     T_PROGRAM = 325,
     T_USE_STATEMENTS = 326,
     T_FQMN = 327,
     T_ARGUMENT_LIST = 328,
     T_LIST = 329,
     T_STATEMENTS = 330,
     T_EXPRESSIONS = 331,
     T_ASSIGNMENT = 332,
     T_FIELDACCESS = 333,
     T_MODIFIERS = 334,
     T_CONSTANTS = 335,
     T_DATA_ELEMENTS = 336,
     T_DATA_STRUCTURE = 337,
     T_DATA_ELEMENT = 338,
     T_METHOD_ARGUMENT = 339
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 55 "compiler/saffire.y"

    char          *sVal;
    long          lVal;
    double        dVal;
    struct ast_element *nPtr;



/* Line 1676 of yacc.c  */
#line 146 "compiler/parser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


