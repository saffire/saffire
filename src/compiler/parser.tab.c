
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 27 "compiler/saffire.y"



#define YY_HEADER_EXPORT_START_CONDITIONS 1

    #include <stdio.h>
    #include "compiler/lex.yy.h"
    #include "compiler/saffire_compiler.h"
    #include "compiler/ast.h"

    extern int yylineno;
    int yylex(void);
    void yyerror(const char *err) { printf("Error in line: %d: %s\n", yylineno, err); }

    void saffire_push_state(int state);
    void saffire_pop_state();

    #ifdef __DEBUG
        #define YYDEBUG 1
        #define TRACE printf("Reduce at line %d\n", __LINE__);
    #else
        #define YYDEBUG 0
        #define TRACE
    #endif

    #define YYPRINT(file, type, value)


/* Line 189 of yacc.c  */
#line 102 "compiler/parser.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 1
#endif


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

/* Line 214 of yacc.c  */
#line 55 "compiler/saffire.y"

    char          *sVal;
    long          lVal;
    double        dVal;
    struct ast_element *nPtr;



/* Line 214 of yacc.c  */
#line 232 "compiler/parser.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 244 "compiler/parser.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  8
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   819

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  109
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  78
/* YYNRULES -- Number of rules.  */
#define YYNRULES  208
/* YYNRULES -- Number of states.  */
#define YYNSTATES  353

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   339

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    94,     2,     2,     2,   105,   104,     2,
      98,    99,    54,    52,   101,    53,   106,    55,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   100,    95,
      44,    42,    43,   102,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,   107,     2,   108,    45,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    96,   103,    97,    93,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    46,    47,    48,
      49,    50,    51,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    11,    13,    16,    22,
      26,    28,    29,    31,    34,    36,    38,    40,    42,    45,
      49,    51,    54,    56,    58,    60,    62,    64,    66,    68,
      72,    78,    84,    90,    94,   100,   107,   115,   121,   123,
     126,   129,   132,   135,   138,   142,   146,   150,   154,   160,
     165,   167,   170,   173,   179,   184,   188,   191,   195,   198,
     200,   202,   206,   208,   212,   214,   220,   222,   226,   228,
     232,   234,   238,   240,   244,   246,   250,   252,   256,   260,
     264,   266,   270,   274,   278,   282,   284,   288,   290,   294,
     298,   300,   304,   308,   310,   314,   318,   322,   325,   328,
     331,   333,   335,   337,   340,   343,   345,   348,   350,   352,
     354,   356,   358,   360,   362,   364,   366,   368,   370,   372,
     374,   376,   378,   380,   382,   384,   386,   388,   392,   394,
     398,   399,   401,   403,   407,   409,   411,   413,   415,   417,
     419,   421,   425,   429,   434,   438,   443,   447,   449,   451,
     453,   456,   458,   460,   462,   464,   467,   469,   471,   473,
     481,   489,   491,   493,   494,   496,   500,   502,   506,   509,
     514,   516,   519,   525,   530,   534,   540,   545,   553,   560,
     567,   573,   580,   585,   590,   592,   595,   597,   599,   601,
     603,   605,   607,   609,   612,   613,   616,   617,   619,   623,
     628,   634,   638,   643,   649,   653,   655,   659,   661
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     110,     0,    -1,   111,    -1,   112,   115,    -1,   113,    -1,
      -1,   114,    -1,   113,   114,    -1,     9,     5,    10,     5,
      95,    -1,     9,     5,    95,    -1,   116,    -1,    -1,   117,
      -1,   116,   117,    -1,   174,    -1,   176,    -1,   172,    -1,
     119,    -1,    96,    97,    -1,    96,   119,    97,    -1,   120,
      -1,   119,   120,    -1,   129,    -1,   118,    -1,   123,    -1,
     121,    -1,   122,    -1,   124,    -1,   125,    -1,     8,   131,
     120,    -1,     8,   131,   120,    16,   120,    -1,    12,    98,
     131,    99,   118,    -1,     7,   131,   120,    16,   120,    -1,
       7,   131,   120,    -1,    11,   120,     7,   131,    95,    -1,
      13,    98,   123,   123,    99,   120,    -1,    13,    98,   123,
     123,   131,    99,   120,    -1,    14,   131,    10,   131,   120,
      -1,    95,    -1,   131,    95,    -1,    30,    95,    -1,    32,
      95,    -1,    33,    95,    -1,    35,    95,    -1,    35,   131,
      95,    -1,    34,   131,    95,    -1,    31,     5,    95,    -1,
      37,   118,   126,    -1,    37,   118,   126,    36,   118,    -1,
      37,   118,    36,   118,    -1,   127,    -1,   126,   127,    -1,
     128,   118,    -1,    29,    98,     5,     5,    99,    -1,    29,
      98,     5,    99,    -1,    29,    98,    99,    -1,     5,   100,
      -1,    15,   130,   100,    -1,    38,   100,    -1,   133,    -1,
     132,    -1,   131,   101,   132,    -1,   133,    -1,   145,   152,
     132,    -1,   134,    -1,   134,   102,   131,   100,   133,    -1,
     135,    -1,   134,    57,   135,    -1,   136,    -1,   135,    56,
     136,    -1,   137,    -1,   136,   103,   137,    -1,   138,    -1,
     137,    45,   138,    -1,   139,    -1,   138,   104,   139,    -1,
     140,    -1,   139,    49,   140,    -1,   139,    48,   140,    -1,
     139,    47,   140,    -1,   141,    -1,   140,    43,   141,    -1,
     140,    44,   141,    -1,   140,    50,   141,    -1,   140,    51,
     141,    -1,   142,    -1,   141,    46,     6,    -1,   143,    -1,
     142,    58,   143,    -1,   142,    59,   143,    -1,   144,    -1,
     143,    52,   144,    -1,   143,    53,   144,    -1,   145,    -1,
     144,    54,   145,    -1,   144,    55,   145,    -1,   144,   105,
     145,    -1,    17,   145,    -1,    18,   145,    -1,   150,   149,
      -1,   148,    -1,   149,    -1,   147,    -1,   146,    17,    -1,
     146,    18,    -1,   146,    -1,   151,   145,    -1,   154,    -1,
     156,    -1,    52,    -1,    53,    -1,    93,    -1,    94,    -1,
      42,    -1,    19,    -1,    20,    -1,    21,    -1,    22,    -1,
      23,    -1,    24,    -1,    25,    -1,    26,    -1,    27,    -1,
      28,    -1,     3,    -1,     4,    -1,     5,    -1,   154,   106,
       5,    -1,   132,    -1,   155,   101,   132,    -1,    -1,   161,
      -1,   157,    -1,    98,   131,    99,    -1,   158,    -1,     3,
      -1,     4,    -1,     6,    -1,   159,    -1,   160,    -1,   184,
      -1,   156,   106,     5,    -1,   154,   106,   161,    -1,   158,
      98,   155,    99,    -1,   158,    98,    99,    -1,   154,    98,
     155,    99,    -1,   154,    98,    99,    -1,    40,    -1,    41,
      -1,   163,    -1,   162,   163,    -1,   173,    -1,   177,    -1,
     168,    -1,   165,    -1,   164,   165,    -1,   166,    -1,   178,
      -1,   167,    -1,   179,    39,     5,    98,   169,    99,    95,
      -1,   179,    39,     5,    98,   169,    99,   118,    -1,   167,
      -1,   170,    -1,    -1,   171,    -1,   170,   101,   171,    -1,
       5,    -1,     5,    42,   149,    -1,     5,     5,    -1,     5,
       5,    42,   149,    -1,   173,    -1,   172,   173,    -1,    69,
       5,    42,   153,    95,    -1,   175,    96,   162,    97,    -1,
     175,    96,    97,    -1,   179,    60,     5,   181,   182,    -1,
      60,     5,   181,   182,    -1,   179,    65,     5,   182,    96,
     164,    97,    -1,   179,    65,     5,   182,    96,    97,    -1,
      65,     5,   182,    96,   164,    97,    -1,    65,     5,   182,
      96,    97,    -1,   179,    72,     5,    42,   131,    95,    -1,
     179,    72,     5,    95,    -1,   179,    72,     5,    95,    -1,
     180,    -1,   179,   180,    -1,    68,    -1,    66,    -1,    67,
      -1,    63,    -1,    62,    -1,    70,    -1,    71,    -1,    61,
       5,    -1,    -1,    64,   183,    -1,    -1,     5,    -1,   183,
     101,     5,    -1,   154,   107,   185,   108,    -1,   154,   107,
     185,   101,   108,    -1,   154,   107,   108,    -1,   157,   107,
     185,   108,    -1,   157,   107,   185,   101,   108,    -1,   157,
     107,   108,    -1,   186,    -1,   185,   101,   186,    -1,   132,
      -1,   186,   100,   132,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   121,   121,   125,   129,   130,   134,   135,   140,   142,
     148,   149,   153,   154,   159,   160,   161,   162,   175,   176,
     180,   181,   185,   186,   187,   188,   189,   190,   191,   195,
     196,   198,   202,   203,   205,   207,   208,   210,   214,   215,
     219,   220,   221,   222,   223,   224,   225,   229,   230,   231,
     235,   236,   240,   244,   245,   246,   250,   251,   252,   263,
     267,   268,   272,   273,   277,   278,   282,   283,   287,   288,
     292,   293,   297,   298,   302,   303,   307,   308,   309,   310,
     314,   315,   316,   317,   318,   322,   323,   327,   328,   329,
     333,   334,   335,   339,   340,   341,   342,   346,   347,   348,
     349,   353,   354,   358,   359,   363,   364,   368,   369,   373,
     374,   378,   379,   385,   386,   387,   388,   389,   390,   391,
     392,   393,   394,   395,   399,   400,   405,   406,   410,   411,
     412,   417,   418,   422,   423,   427,   428,   429,   430,   431,
     432,   436,   437,   441,   442,   443,   444,   448,   449,   462,
     463,   467,   468,   469,   474,   475,   479,   480,   484,   488,
     492,   493,   497,   498,   502,   503,   507,   508,   509,   510,
     514,   515,   519,   523,   524,   528,   529,   533,   534,   535,
     536,   540,   541,   545,   549,   550,   555,   556,   557,   558,
     559,   560,   561,   566,   567,   572,   573,   578,   579,   591,
     592,   593,   594,   595,   596,   600,   601,   605,   606
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "$undefined", "T_LNUM", "T_STRING",
  "T_IDENTIFIER", "T_REGEX", "T_WHILE", "T_IF", "T_USE", "T_AS", "T_DO",
  "T_SWITCH", "T_FOR", "T_FOREACH", "T_CASE", "T_ELSE", "T_OP_INC",
  "T_OP_DEC", "T_PLUS_ASSIGNMENT", "T_MINUS_ASSIGNMENT",
  "T_MUL_ASSIGNMENT", "T_DIV_ASSIGNMENT", "T_MOD_ASSIGNMENT",
  "T_AND_ASSIGNMENT", "T_OR_ASSIGNMENT", "T_XOR_ASSIGNMENT",
  "T_SL_ASSIGNMENT", "T_SR_ASSIGNMENT", "T_CATCH", "T_BREAK", "T_GOTO",
  "T_BREAKELSE", "T_CONTINUE", "T_THROW", "T_RETURN", "T_FINALLY", "T_TRY",
  "T_DEFAULT", "T_METHOD", "T_SELF", "T_PARENT", "'='", "'>'", "'<'",
  "'^'", "T_RE", "T_IN", "T_NE", "T_EQ", "T_LE", "T_GE", "'+'", "'-'",
  "'*'", "'/'", "T_AND", "T_OR", "T_SHIFT_LEFT", "T_SHIFT_RIGHT",
  "T_CLASS", "T_EXTENDS", "T_ABSTRACT", "T_FINAL", "T_IMPLEMENTS",
  "T_INTERFACE", "T_PUBLIC", "T_PRIVATE", "T_PROTECTED", "T_CONST",
  "T_STATIC", "T_READONLY", "T_PROPERTY", "T_LABEL", "T_METHOD_CALL",
  "T_ARITHMIC", "T_LOGICAL", "T_TOP_STATEMENTS", "T_PROGRAM",
  "T_USE_STATEMENTS", "T_FQMN", "T_ARGUMENT_LIST", "T_LIST",
  "T_STATEMENTS", "T_EXPRESSIONS", "T_ASSIGNMENT", "T_FIELDACCESS",
  "T_MODIFIERS", "T_CONSTANTS", "T_DATA_ELEMENTS", "T_DATA_STRUCTURE",
  "T_DATA_ELEMENT", "T_METHOD_ARGUMENT", "'~'", "'!'", "';'", "'{'", "'}'",
  "'('", "')'", "':'", "','", "'?'", "'|'", "'&'", "'%'", "'.'", "'['",
  "']'", "$accept", "saffire", "program", "use_statement_list",
  "non_empty_use_statement_list", "use_statement", "top_statement_list",
  "non_empty_top_statement_list", "top_statement", "compound_statement",
  "statement_list", "statement", "selection_statement",
  "iteration_statement", "expression_statement", "jump_statement",
  "guarding_statement", "catch_list", "catch", "catch_header",
  "label_statement", "constant_expression", "expression",
  "assignment_expression", "conditional_expression",
  "conditional_or_expression", "conditional_and_expression",
  "inclusive_or_expression", "exclusive_or_expression", "and_expression",
  "equality_expression", "relational_expression", "regex_expression",
  "shift_expression", "additive_expression", "multiplicative_expression",
  "unary_expression", "postfix_expression", "real_postfix_expression",
  "logical_unary_expression", "primary_expression",
  "arithmic_unary_operator", "logical_unary_operator",
  "assignment_operator", "real_scalar_value", "qualified_name",
  "calling_method_argument_list", "not_just_name", "complex_primary",
  "complex_primary_no_parenthesis", "field_access", "method_call",
  "special_name", "class_inner_statement_list", "class_inner_statement",
  "interface_inner_statement_list", "interface_inner_statement",
  "interface_method_definition", "interface_or_abstract_method_definition",
  "class_method_definition", "method_argument_list",
  "non_empty_method_argument_list", "method_argument", "constant_list",
  "constant", "class_definition", "class_header", "interface_definition",
  "class_property_definition", "interface_property_definition",
  "modifier_list", "modifier", "class_extends",
  "class_interface_implements", "class_list", "data_structure",
  "data_structure_elements", "data_structure_element", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,    61,    62,    60,    94,   297,   298,   299,   300,
     301,   302,    43,    45,    42,    47,   303,   304,   305,   306,
     307,   308,   309,   310,   311,   312,   313,   314,   315,   316,
     317,   318,   319,   320,   321,   322,   323,   324,   325,   326,
     327,   328,   329,   330,   331,   332,   333,   334,   335,   336,
     337,   338,   339,   126,    33,    59,   123,   125,    40,    41,
      58,    44,    63,   124,    38,    37,    46,    91,    93
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   109,   110,   111,   112,   112,   113,   113,   114,   114,
     115,   115,   116,   116,   117,   117,   117,   117,   118,   118,
     119,   119,   120,   120,   120,   120,   120,   120,   120,   121,
     121,   121,   122,   122,   122,   122,   122,   122,   123,   123,
     124,   124,   124,   124,   124,   124,   124,   125,   125,   125,
     126,   126,   127,   128,   128,   128,   129,   129,   129,   130,
     131,   131,   132,   132,   133,   133,   134,   134,   135,   135,
     136,   136,   137,   137,   138,   138,   139,   139,   139,   139,
     140,   140,   140,   140,   140,   141,   141,   142,   142,   142,
     143,   143,   143,   144,   144,   144,   144,   145,   145,   145,
     145,   146,   146,   147,   147,   148,   148,   149,   149,   150,
     150,   151,   151,   152,   152,   152,   152,   152,   152,   152,
     152,   152,   152,   152,   153,   153,   154,   154,   155,   155,
     155,   156,   156,   157,   157,   158,   158,   158,   158,   158,
     158,   159,   159,   160,   160,   160,   160,   161,   161,   162,
     162,   163,   163,   163,   164,   164,   165,   165,   166,   167,
     168,   168,   169,   169,   170,   170,   171,   171,   171,   171,
     172,   172,   173,   174,   174,   175,   175,   176,   176,   176,
     176,   177,   177,   178,   179,   179,   180,   180,   180,   180,
     180,   180,   180,   181,   181,   182,   182,   183,   183,   184,
     184,   184,   184,   184,   184,   185,   185,   186,   186
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     0,     1,     2,     5,     3,
       1,     0,     1,     2,     1,     1,     1,     1,     2,     3,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     3,
       5,     5,     5,     3,     5,     6,     7,     5,     1,     2,
       2,     2,     2,     2,     3,     3,     3,     3,     5,     4,
       1,     2,     2,     5,     4,     3,     2,     3,     2,     1,
       1,     3,     1,     3,     1,     5,     1,     3,     1,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     3,     3,
       1,     3,     3,     3,     3,     1,     3,     1,     3,     3,
       1,     3,     3,     1,     3,     3,     3,     2,     2,     2,
       1,     1,     1,     2,     2,     1,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     1,     3,
       0,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     4,     3,     4,     3,     1,     1,     1,
       2,     1,     1,     1,     1,     2,     1,     1,     1,     7,
       7,     1,     1,     0,     1,     3,     1,     3,     2,     4,
       1,     2,     5,     4,     3,     5,     4,     7,     6,     6,
       5,     6,     4,     4,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     2,     0,     2,     0,     1,     3,     4,
       5,     3,     4,     5,     3,     1,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       5,     0,     0,     2,    11,     4,     6,     0,     1,   135,
     136,   126,   137,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     147,   148,   109,   110,     0,   190,   189,     0,   187,   188,
     186,     0,   191,   192,   111,   112,    38,     0,     0,     3,
      10,    12,    23,    17,    20,    25,    26,    24,    27,    28,
      22,     0,    60,    62,    64,    66,    68,    70,    72,    74,
      76,    80,    85,    87,    90,    93,   105,   102,   100,   101,
       0,     0,   107,   108,   132,   134,   138,   139,   131,    16,
     170,    14,     0,    15,     0,   184,   140,     7,     0,     9,
      56,   126,     0,     0,     0,     0,     0,     0,     0,    59,
      93,    97,    98,    40,     0,    41,    42,     0,    43,     0,
       0,    58,   194,   196,     0,    18,     0,     0,    13,    21,
      39,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   113,     0,   103,   104,    99,   106,   130,
       0,     0,     0,     0,   130,   171,     0,     0,     0,   185,
       0,    33,    29,     0,     0,     0,     0,    57,    46,    45,
      44,     0,     0,    47,    50,     0,     0,   196,     0,     0,
       0,    19,   133,    61,    67,     0,    69,    71,    73,    75,
      79,    78,    77,    81,    82,    83,    84,    86,    88,    89,
      91,    92,    94,    95,    96,    63,   146,   128,     0,   127,
     142,   201,   207,     0,   205,   141,   204,     0,   144,     0,
     174,     0,   149,   161,   153,   151,   152,     0,   194,   196,
       8,     0,     0,     0,     0,     0,     0,     0,    49,     0,
      51,    52,   193,   176,   197,   195,     0,   124,   125,     0,
       0,   145,     0,     0,   199,     0,     0,   202,   143,   173,
     150,     0,     0,   196,     0,    32,    30,    34,    31,     0,
       0,    37,     0,    55,    48,     0,   180,     0,   154,   156,
     158,   157,     0,   172,    65,   129,   200,   206,   208,   203,
       0,     0,   175,     0,    35,     0,     0,    54,   198,   179,
     155,     0,     0,   163,     0,   182,   178,     0,    36,    53,
       0,     0,   166,     0,   162,   164,     0,   177,   163,   183,
     168,     0,     0,     0,   181,     0,     0,   167,   159,   160,
     165,     0,   169
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     4,     5,     6,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,   193,   194,   195,
      60,   108,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,   164,   269,    82,   228,    83,    84,    85,
      86,    87,    88,   241,   242,   297,   298,   299,   300,   244,
     333,   334,   335,    89,    90,    91,    92,    93,   246,   301,
     302,    95,   197,   199,   265,    96,   233,   234
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -252
static const yytype_int16 yypact[] =
{
      42,    62,    79,  -252,   491,    42,  -252,     9,  -252,  -252,
    -252,    -2,  -252,   448,   448,   662,    -4,    14,   448,   448,
     448,   448,     6,   102,    40,    48,   448,   713,    56,    55,
    -252,  -252,  -252,  -252,   115,  -252,  -252,   158,  -252,  -252,
    -252,   161,  -252,  -252,  -252,  -252,  -252,   560,   448,  -252,
     491,  -252,  -252,   662,  -252,  -252,  -252,  -252,  -252,  -252,
    -252,   -17,  -252,  -252,   -25,   124,    84,   166,   116,   121,
      13,   173,    24,    64,   -15,   756,   155,  -252,  -252,  -252,
      25,   448,   -63,   117,   120,   123,  -252,  -252,  -252,   153,
    -252,  -252,   132,  -252,   725,  -252,  -252,  -252,   224,  -252,
    -252,  -252,   382,   382,   225,   448,   721,     8,   136,  -252,
    -252,  -252,  -252,  -252,   138,  -252,  -252,    -6,  -252,     1,
      39,  -252,   176,   174,   200,  -252,   611,   -52,  -252,  -252,
    -252,   448,   448,   448,   448,   448,   448,   448,   448,   448,
     448,   448,   448,   448,   448,   242,   448,   448,   448,   448,
     448,   448,   448,  -252,  -252,  -252,  -252,  -252,  -252,  -252,
    -252,  -252,  -252,  -252,   448,  -252,  -252,  -252,  -252,    20,
      29,   141,   252,   172,   312,  -252,   133,   253,   254,  -252,
     167,   245,   247,   448,    61,   721,   448,  -252,  -252,  -252,
    -252,   169,    56,    51,  -252,    56,   263,   174,   270,   180,
     188,  -252,  -252,  -252,   124,    97,    84,   166,   116,   121,
      13,    13,    13,   173,   173,   173,   173,  -252,    64,    64,
     -15,   -15,  -252,  -252,  -252,  -252,  -252,  -252,    87,  -252,
    -252,  -252,  -252,   -60,   177,  -252,  -252,   -20,  -252,   106,
    -252,   184,  -252,  -252,  -252,  -252,  -252,   288,   176,   174,
    -252,   662,   662,     2,    56,   421,   382,    11,  -252,    56,
    -252,  -252,  -252,  -252,  -252,   178,   229,  -252,  -252,   187,
     448,  -252,   448,   268,  -252,   448,   284,  -252,  -252,  -252,
    -252,   278,   279,   174,   197,  -252,  -252,  -252,  -252,   662,
     107,  -252,    12,  -252,  -252,   289,  -252,   272,  -252,  -252,
    -252,  -252,   674,  -252,  -252,  -252,  -252,   177,  -252,  -252,
     205,    -9,  -252,   374,  -252,   662,   199,  -252,  -252,  -252,
    -252,   300,   301,   302,   448,  -252,  -252,   540,  -252,  -252,
     212,   216,    16,   214,   218,  -252,     4,  -252,   302,  -252,
     280,    25,   114,   302,  -252,   232,    25,  -252,  -252,  -252,
    -252,   228,  -252
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -252,  -252,  -252,  -252,  -252,   323,  -252,  -252,   282,   -28,
     286,   -11,  -252,  -252,   -79,  -252,  -252,  -252,   148,  -252,
    -252,  -252,   -12,  -119,   -14,  -252,   213,   210,   211,   221,
     226,    45,     7,  -252,    68,    69,   -10,  -252,  -252,  -252,
     -77,  -252,  -252,  -252,  -252,  -252,   175,  -252,  -252,  -252,
    -252,  -252,   198,  -252,   126,    34,  -251,  -252,  -156,  -252,
      10,  -252,    27,  -252,   -76,  -252,  -252,  -252,  -252,  -252,
       3,   -86,   125,  -175,  -252,  -252,   201,  -214
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
     120,   102,   103,   167,   104,   109,   107,    94,   179,   110,
     111,   112,   203,   175,   117,   119,   292,   316,   186,    98,
     243,   340,   263,     9,    10,   101,    12,   185,     9,    10,
     101,    12,   132,   324,   229,   169,   127,    20,    21,   150,
     151,   273,   129,   170,   171,   225,   320,   202,   274,   131,
     227,     1,   232,    94,   232,   227,   141,   142,   341,   307,
      30,    31,   307,   143,   144,    30,    31,     7,   191,    30,
      31,   168,    32,    33,   284,   192,   320,   133,   130,     8,
     191,   276,   146,   147,   131,   243,   325,   259,   277,   189,
     152,   181,   182,   184,   105,   131,   190,   287,   100,   344,
     245,   113,   131,   131,    99,   131,   255,   114,   312,   131,
     293,   317,   106,    44,    45,   129,   148,   149,    48,   226,
     122,   205,   110,    48,   110,   110,   110,   110,   110,   110,
     110,   110,   110,   110,   110,   115,   110,   110,   110,   110,
     222,   223,   224,   116,     9,    10,   101,    12,   213,   214,
     215,   216,    47,   305,   232,   121,   308,   232,    20,    21,
     254,   179,   131,   123,   258,   245,   124,   261,   138,   139,
     140,   253,   165,   166,   256,     9,    10,   101,    12,   247,
     134,    30,    31,   210,   211,   212,   271,   135,   272,    20,
      21,   267,   268,    32,    33,    35,    36,   270,   131,    38,
      39,    40,    41,    42,    43,   278,   315,   272,   131,   348,
      47,   136,    30,    31,   218,   219,   179,   220,   221,   145,
     137,   174,    41,   172,    32,    33,   288,   173,   176,   180,
     240,   294,   183,   188,    44,    45,   187,   196,   198,    48,
     285,   286,   200,   290,   247,   291,    35,    36,   217,   231,
      38,    39,    40,    41,    42,    43,   304,   235,   248,   249,
     110,   251,   250,   252,   347,    44,    45,   257,   262,   352,
      48,     9,    10,   101,    12,   264,   266,   275,   314,   295,
     236,   279,   303,   310,   311,    20,    21,     9,    10,   101,
      12,    35,    36,   313,   318,    38,    39,    40,   329,    42,
      43,    20,    21,   323,   328,   330,   331,   332,    30,    31,
     338,   339,   336,   342,   349,     9,    10,   101,    12,   343,
      32,    33,   346,   348,    30,    31,   296,   281,    97,    20,
      21,   351,   128,   126,    35,    36,    32,    33,    38,    39,
      40,   260,    42,    43,   206,   204,   207,   327,   345,   239,
      35,    36,    30,    31,    38,    39,    40,   208,    42,    43,
     282,    44,    45,   209,    32,    33,    48,   280,   230,   319,
     350,     0,     0,   283,   237,     0,   306,    44,    45,     0,
       0,     0,    48,     0,     0,     9,    10,    11,    12,    13,
      14,     0,   309,    15,    16,    17,    18,    19,     0,    20,
      21,     0,     0,     0,     0,    44,    45,     0,     0,     0,
      48,   238,    22,    23,    24,    25,    26,    27,     0,    28,
      29,     0,    30,    31,     9,    10,   101,    12,     0,     0,
       0,     0,     0,     0,    32,    33,    35,    36,    20,    21,
      38,    39,    40,     0,    42,    43,     0,     0,     0,     0,
       0,     9,    10,   101,    12,     0,     0,     0,     0,     0,
       0,    30,    31,     0,     0,    20,    21,     0,     0,     0,
       0,   326,     0,    32,    33,    44,    45,    46,    47,     0,
      48,     0,     0,   131,     0,     0,     0,     0,    30,    31,
       0,     0,     0,     0,     9,    10,    11,    12,    13,    14,
      32,    33,    15,    16,    17,    18,    19,     0,    20,    21,
       0,     0,     0,     0,    44,    45,     0,     0,     0,    48,
     289,    22,    23,    24,    25,    26,    27,     0,    28,    29,
       0,    30,    31,     0,     0,     0,     0,     0,     0,     0,
       0,    44,    45,    32,    33,     0,    48,     0,     0,     0,
       0,    34,     0,    35,    36,     0,    37,    38,    39,    40,
      41,    42,    43,     9,    10,    11,    12,    13,    14,     0,
       0,    15,    16,    17,    18,    19,     0,    20,    21,     0,
       0,     0,     0,     0,    44,    45,    46,    47,     0,    48,
      22,    23,    24,    25,    26,    27,     0,    28,    29,     0,
      30,    31,    35,    36,     0,     0,    38,    39,    40,     0,
      42,    43,    32,    33,     9,    10,    11,    12,    13,    14,
       0,     0,    15,    16,    17,    18,    19,     0,    20,    21,
       0,     0,     0,     0,     0,     0,     0,   337,     0,     0,
       0,    22,    23,    24,    25,    26,    27,     0,    28,    29,
       0,    30,    31,    44,    45,    46,    47,   125,    48,     0,
       0,     0,     0,    32,    33,     9,    10,    11,    12,    13,
      14,     0,     0,    15,    16,    17,    18,    19,     0,    20,
      21,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    22,    23,    24,    25,    26,    27,     0,    28,
      29,     0,    30,    31,    44,    45,    46,    47,   201,    48,
       0,     0,     0,   321,    32,    33,     9,    10,   101,    12,
       0,     0,     0,     0,     9,    10,   101,    12,     0,     0,
      20,    21,     0,     0,     0,     0,    35,    36,    20,    21,
      38,    39,    40,     0,    42,    43,   322,     0,     0,     0,
       0,     0,     0,    30,    31,    44,    45,    46,    47,     0,
      48,    30,    31,     0,     0,    32,    33,     0,     0,     0,
       0,     0,     0,    32,    33,   153,   154,   155,   156,   157,
     158,   159,   160,   161,   162,   177,     0,    35,    36,     0,
     178,    38,    39,    40,     0,    42,    43,     0,   163,     0,
       0,     0,     0,     0,     0,     0,    44,    45,   118,     0,
       0,    48,     0,     0,    44,    45,    46,     0,     0,    48
};

static const yytype_int16 yycheck[] =
{
      28,    13,    14,    80,    15,    19,    18,     4,    94,    19,
      20,    21,   131,    89,    26,    27,     5,     5,    10,    10,
     176,     5,   197,     3,     4,     5,     6,   106,     3,     4,
       5,     6,    57,    42,     5,    98,    48,    17,    18,    54,
      55,   101,    53,   106,   107,   164,   297,    99,   108,   101,
     169,     9,   171,    50,   173,   174,    43,    44,    42,   273,
      40,    41,   276,    50,    51,    40,    41,     5,    29,    40,
      41,    81,    52,    53,   249,    36,   327,   102,    95,     0,
      29,   101,    58,    59,   101,   241,    95,    36,   108,    95,
     105,   102,   103,   105,    98,   101,    95,    95,   100,    95,
     176,    95,   101,   101,    95,   101,   185,     5,   283,   101,
      99,    99,    98,    93,    94,   126,    52,    53,    98,    99,
       5,   133,   132,    98,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,    95,   146,   147,   148,   149,
     150,   151,   152,    95,     3,     4,     5,     6,   141,   142,
     143,   144,    96,   272,   273,   100,   275,   276,    17,    18,
      99,   247,   101,     5,   192,   241,     5,   195,    47,    48,
      49,   183,    17,    18,   186,     3,     4,     5,     6,   176,
      56,    40,    41,   138,   139,   140,    99,   103,   101,    17,
      18,     3,     4,    52,    53,    62,    63,   100,   101,    66,
      67,    68,    69,    70,    71,    99,    99,   101,   101,    95,
      96,    45,    40,    41,   146,   147,   302,   148,   149,    46,
     104,    98,    69,   106,    52,    53,   254,   107,    96,     5,
      97,   259,     7,    95,    93,    94,   100,    61,    64,    98,
     251,   252,    42,   255,   241,   256,    62,    63,     6,   108,
      66,    67,    68,    69,    70,    71,   270,     5,     5,     5,
     270,    16,    95,    16,   341,    93,    94,    98,     5,   346,
      98,     3,     4,     5,     6,     5,    96,   100,   289,   101,
     108,    97,    95,     5,     5,    17,    18,     3,     4,     5,
       6,    62,    63,    96,     5,    66,    67,    68,    99,    70,
      71,    17,    18,    98,   315,     5,     5,     5,    40,    41,
      98,    95,   324,    99,   342,     3,     4,     5,     6,   101,
      52,    53,    42,    95,    40,    41,    97,    39,     5,    17,
      18,    99,    50,    47,    62,    63,    52,    53,    66,    67,
      68,   193,    70,    71,   134,   132,   135,   313,   338,   174,
      62,    63,    40,    41,    66,    67,    68,   136,    70,    71,
      72,    93,    94,   137,    52,    53,    98,   241,   170,    97,
     343,    -1,    -1,   248,   173,    -1,   108,    93,    94,    -1,
      -1,    -1,    98,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,   108,    11,    12,    13,    14,    15,    -1,    17,
      18,    -1,    -1,    -1,    -1,    93,    94,    -1,    -1,    -1,
      98,    99,    30,    31,    32,    33,    34,    35,    -1,    37,
      38,    -1,    40,    41,     3,     4,     5,     6,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    53,    62,    63,    17,    18,
      66,    67,    68,    -1,    70,    71,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,    -1,    -1,    -1,    -1,    -1,
      -1,    40,    41,    -1,    -1,    17,    18,    -1,    -1,    -1,
      -1,    97,    -1,    52,    53,    93,    94,    95,    96,    -1,
      98,    -1,    -1,   101,    -1,    -1,    -1,    -1,    40,    41,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,     8,
      52,    53,    11,    12,    13,    14,    15,    -1,    17,    18,
      -1,    -1,    -1,    -1,    93,    94,    -1,    -1,    -1,    98,
      99,    30,    31,    32,    33,    34,    35,    -1,    37,    38,
      -1,    40,    41,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    93,    94,    52,    53,    -1,    98,    -1,    -1,    -1,
      -1,    60,    -1,    62,    63,    -1,    65,    66,    67,    68,
      69,    70,    71,     3,     4,     5,     6,     7,     8,    -1,
      -1,    11,    12,    13,    14,    15,    -1,    17,    18,    -1,
      -1,    -1,    -1,    -1,    93,    94,    95,    96,    -1,    98,
      30,    31,    32,    33,    34,    35,    -1,    37,    38,    -1,
      40,    41,    62,    63,    -1,    -1,    66,    67,    68,    -1,
      70,    71,    52,    53,     3,     4,     5,     6,     7,     8,
      -1,    -1,    11,    12,    13,    14,    15,    -1,    17,    18,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    97,    -1,    -1,
      -1,    30,    31,    32,    33,    34,    35,    -1,    37,    38,
      -1,    40,    41,    93,    94,    95,    96,    97,    98,    -1,
      -1,    -1,    -1,    52,    53,     3,     4,     5,     6,     7,
       8,    -1,    -1,    11,    12,    13,    14,    15,    -1,    17,
      18,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    30,    31,    32,    33,    34,    35,    -1,    37,
      38,    -1,    40,    41,    93,    94,    95,    96,    97,    98,
      -1,    -1,    -1,    39,    52,    53,     3,     4,     5,     6,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    -1,    -1,
      17,    18,    -1,    -1,    -1,    -1,    62,    63,    17,    18,
      66,    67,    68,    -1,    70,    71,    72,    -1,    -1,    -1,
      -1,    -1,    -1,    40,    41,    93,    94,    95,    96,    -1,
      98,    40,    41,    -1,    -1,    52,    53,    -1,    -1,    -1,
      -1,    -1,    -1,    52,    53,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    60,    -1,    62,    63,    -1,
      65,    66,    67,    68,    -1,    70,    71,    -1,    42,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    93,    94,    95,    -1,
      -1,    98,    -1,    -1,    93,    94,    95,    -1,    -1,    98
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     9,   110,   111,   112,   113,   114,     5,     0,     3,
       4,     5,     6,     7,     8,    11,    12,    13,    14,    15,
      17,    18,    30,    31,    32,    33,    34,    35,    37,    38,
      40,    41,    52,    53,    60,    62,    63,    65,    66,    67,
      68,    69,    70,    71,    93,    94,    95,    96,    98,   115,
     116,   117,   118,   119,   120,   121,   122,   123,   124,   125,
     129,   131,   132,   133,   134,   135,   136,   137,   138,   139,
     140,   141,   142,   143,   144,   145,   146,   147,   148,   149,
     150,   151,   154,   156,   157,   158,   159,   160,   161,   172,
     173,   174,   175,   176,   179,   180,   184,   114,    10,    95,
     100,     5,   131,   131,   120,    98,    98,   131,   130,   133,
     145,   145,   145,    95,     5,    95,    95,   131,    95,   131,
     118,   100,     5,     5,     5,    97,   119,   131,   117,   120,
      95,   101,    57,   102,    56,   103,    45,   104,    47,    48,
      49,    43,    44,    50,    51,    46,    58,    59,    52,    53,
      54,    55,   105,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    42,   152,    17,    18,   149,   145,    98,
     106,   107,   106,   107,    98,   173,    96,    60,    65,   180,
       5,   120,   120,     7,   131,   123,    10,   100,    95,    95,
      95,    29,    36,   126,   127,   128,    61,   181,    64,   182,
      42,    97,    99,   132,   135,   131,   136,   137,   138,   139,
     140,   140,   140,   141,   141,   141,   141,     6,   143,   143,
     144,   144,   145,   145,   145,   132,    99,   132,   155,     5,
     161,   108,   132,   185,   186,     5,   108,   185,    99,   155,
      97,   162,   163,   167,   168,   173,   177,   179,     5,     5,
      95,    16,    16,   131,    99,   123,   131,    98,   118,    36,
     127,   118,     5,   182,     5,   183,    96,     3,     4,   153,
     100,    99,   101,   101,   108,   100,   101,   108,    99,    97,
     163,    39,    72,   181,   182,   120,   120,    95,   118,    99,
     131,   120,     5,    99,   118,   101,    97,   164,   165,   166,
     167,   178,   179,    95,   133,   132,   108,   186,   132,   108,
       5,     5,   182,    96,   120,    99,     5,    99,     5,    97,
     165,    39,    72,    98,    42,    95,    97,   164,   120,    99,
       5,     5,     5,   169,   170,   171,   131,    97,    98,    95,
       5,    42,    99,   101,    95,   169,    42,   149,    95,   118,
     171,    99,   149
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 121 "compiler/saffire.y"
    { TRACE ast_root = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 3:

/* Line 1455 of yacc.c  */
#line 125 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_PROGRAM,2, (yyvsp[(1) - (2)].nPtr), (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 129 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 130 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_nop(); ;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 134 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_USE_STATEMENTS, 1, (yyvsp[(1) - (1)].nPtr)); ;}
    break;

  case 7:

/* Line 1455 of yacc.c  */
#line 135 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 8:

/* Line 1455 of yacc.c  */
#line 140 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_USE, 2, ast_string((yyvsp[(2) - (5)].sVal)), ast_string((yyvsp[(4) - (5)].sVal))); ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 142 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_USE, 2, ast_string((yyvsp[(2) - (3)].sVal)), ast_string((yyvsp[(2) - (3)].sVal))); ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 148 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 149 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_nop(); ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 153 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_TOP_STATEMENTS, 1, (yyvsp[(1) - (1)].nPtr)); ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 154 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 159 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 160 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 161 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 162 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 175 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_nop(); ;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 176 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(2) - (3)].nPtr); ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 180 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_STATEMENTS, 1, (yyvsp[(1) - (1)].nPtr)); ;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 181 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 185 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 186 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 187 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 25:

/* Line 1455 of yacc.c  */
#line 188 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 189 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 190 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 191 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 195 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_IF, 2, (yyvsp[(2) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 196 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_IF, 3, (yyvsp[(2) - (5)].nPtr), (yyvsp[(3) - (5)].nPtr), (yyvsp[(5) - (5)].nPtr)); ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 198 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_SWITCH, 2, (yyvsp[(3) - (5)].nPtr), (yyvsp[(5) - (5)].nPtr)); ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 202 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_WHILE, 3, (yyvsp[(2) - (5)].nPtr), (yyvsp[(3) - (5)].nPtr), (yyvsp[(5) - (5)].nPtr)); ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 203 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_WHILE, 2, (yyvsp[(2) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 205 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_DO, 2, (yyvsp[(2) - (5)].nPtr), (yyvsp[(4) - (5)].nPtr)); ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 207 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_FOR, 3, (yyvsp[(3) - (6)].nPtr), (yyvsp[(4) - (6)].nPtr), (yyvsp[(6) - (6)].nPtr)); ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 208 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_FOR, 4, (yyvsp[(3) - (7)].nPtr), (yyvsp[(4) - (7)].nPtr), (yyvsp[(5) - (7)].nPtr), (yyvsp[(7) - (7)].nPtr)); ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 210 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_FOREACH, 3, (yyvsp[(2) - (5)].nPtr), (yyvsp[(4) - (5)].nPtr), (yyvsp[(5) - (5)].nPtr)); ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 214 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(';', 0); ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 215 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (2)].nPtr); ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 219 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_BREAK, 0); ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 220 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_BREAKELSE, 0); ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 221 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_CONTINUE, 0); ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 222 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_RETURN, 0); ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 223 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_RETURN, 1, (yyvsp[(2) - (3)].nPtr)); ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 224 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_THROW, 1, (yyvsp[(2) - (3)].nPtr)); ;}
    break;

  case 46:

/* Line 1455 of yacc.c  */
#line 225 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_GOTO, 1, ast_string((yyvsp[(2) - (3)].sVal))); ;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 229 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_TRY, 2, (yyvsp[(2) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 230 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_FINALLY, 3, (yyvsp[(2) - (5)].nPtr), (yyvsp[(3) - (5)].nPtr), (yyvsp[(5) - (5)].nPtr)); ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 231 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_FINALLY, 2, (yyvsp[(2) - (4)].nPtr), (yyvsp[(4) - (4)].nPtr)); ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 235 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr) ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 236 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_LIST, 2, (yyvsp[(1) - (2)].nPtr), (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 240 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_LIST, 2, (yyvsp[(1) - (2)].nPtr), (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 244 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_CATCH, 2, ast_string((yyvsp[(3) - (5)].sVal)), ast_string((yyvsp[(4) - (5)].sVal))); ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 245 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_CATCH, 1, ast_string((yyvsp[(3) - (4)].sVal))); ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 246 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_CATCH, 0); ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 250 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_LABEL, 1, ast_string((yyvsp[(1) - (2)].sVal))); ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 251 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_CASE, 1, (yyvsp[(2) - (3)].nPtr)); ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 252 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_DEFAULT, 0); ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 263 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 267 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_EXPRESSIONS, 1, (yyvsp[(1) - (1)].nPtr)); ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 268 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 272 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 273 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_ASSIGNMENT, 3, (yyvsp[(1) - (3)].nPtr), (yyvsp[(2) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 277 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 278 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('?', 2, (yyvsp[(1) - (5)].nPtr), (yyvsp[(3) - (5)].nPtr)); ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 282 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 283 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_OR, 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr));;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 287 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 288 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_AND, 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 292 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 293 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('|', 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr));;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 297 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 298 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('^', 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 302 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 303 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('&', 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 307 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 308 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_EQ, 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 309 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_NE, 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 310 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_IN, 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 314 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 315 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('>', 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 316 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('<', 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 317 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_LE, 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 318 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_GE, 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 322 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 323 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_RE, 2, (yyvsp[(1) - (3)].nPtr), ast_string((yyvsp[(3) - (3)].sVal))); ;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 327 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 328 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_SHIFT_LEFT, 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 329 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_SHIFT_RIGHT, 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 333 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 334 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('+', 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 335 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('-', 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 339 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 340 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('*', 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 341 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('/', 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 342 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('%', 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 346 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_OP_INC, 1, (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 347 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_OP_DEC, 1, (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 348 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_ARITHMIC, 2, (yyvsp[(1) - (2)].nPtr), (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 349 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 353 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 354 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 358 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_OP_INC, 1, (yyvsp[(1) - (2)].nPtr)); ;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 359 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_OP_DEC, 1, (yyvsp[(1) - (2)].nPtr)); ;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 363 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 364 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_LOGICAL, 2, (yyvsp[(1) - (2)].nPtr), (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 368 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 369 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 373 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("+"); ;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 374 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("-"); ;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 378 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("~"); ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 379 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("!"); ;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 385 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("=");  ;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 386 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("+="); ;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 387 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("-="); ;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 388 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("*="); ;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 389 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("/="); ;}
    break;

  case 118:

/* Line 1455 of yacc.c  */
#line 390 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("%="); ;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 391 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("&="); ;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 392 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("|="); ;}
    break;

  case 121:

/* Line 1455 of yacc.c  */
#line 393 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("^="); ;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 394 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("<="); ;}
    break;

  case 123:

/* Line 1455 of yacc.c  */
#line 395 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string(">="); ;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 399 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_numerical((yyvsp[(1) - (1)].lVal)); ;}
    break;

  case 125:

/* Line 1455 of yacc.c  */
#line 400 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string((yyvsp[(1) - (1)].sVal)); ;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 405 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_FQMN, 1, ast_identifier((yyvsp[(1) - (1)].sVal))); ;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 406 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), ast_identifier((yyvsp[(3) - (3)].sVal))); ;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 410 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_ARGUMENT_LIST, 1, (yyvsp[(1) - (1)].nPtr)); ;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 411 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 412 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_nop(); ;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 417 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 418 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 422 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(2) - (3)].nPtr); ;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 423 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 427 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_numerical((yyvsp[(1) - (1)].lVal)); ;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 428 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string((yyvsp[(1) - (1)].sVal)); ;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 429 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string((yyvsp[(1) - (1)].sVal)); ;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 430 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 431 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 432 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 436 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('.', 2, (yyvsp[(1) - (3)].nPtr), ast_identifier((yyvsp[(3) - (3)].sVal))); ;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 437 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr('.', 2, (yyvsp[(1) - (3)].nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 441 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_METHOD_CALL, 2, (yyvsp[(1) - (4)].nPtr), (yyvsp[(3) - (4)].nPtr)); ;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 442 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_METHOD_CALL, 1, (yyvsp[(1) - (3)].nPtr)); ;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 443 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_METHOD_CALL, 2, (yyvsp[(1) - (4)].nPtr), (yyvsp[(3) - (4)].nPtr)); ;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 444 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_METHOD_CALL, 1, (yyvsp[(1) - (3)].nPtr)); ;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 448 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("SELF"); ;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 449 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string("PARENT"); ;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 462 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_STATEMENTS, 1, (yyvsp[(1) - (1)].nPtr)); ;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 463 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 467 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 468 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 153:

/* Line 1455 of yacc.c  */
#line 469 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 474 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_STATEMENTS, 1, (yyvsp[(1) - (1)].nPtr)); ;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 475 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 479 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 480 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 484 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 488 "compiler/saffire.y"
    { TRACE sfc_validate_method_modifiers((yyvsp[(1) - (7)].lVal)); (yyval.nPtr) = ast_method((yyvsp[(1) - (7)].lVal), (yyvsp[(3) - (7)].sVal), (yyvsp[(5) - (7)].nPtr), ast_nop()); ;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 492 "compiler/saffire.y"
    { TRACE sfc_validate_abstract_method_body((yyvsp[(1) - (7)].lVal), (yyvsp[(7) - (7)].nPtr)); sfc_validate_method_modifiers((yyvsp[(1) - (7)].lVal)); (yyval.nPtr) = ast_method((yyvsp[(1) - (7)].lVal), (yyvsp[(3) - (7)].sVal), (yyvsp[(5) - (7)].nPtr), (yyvsp[(7) - (7)].nPtr)); ;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 493 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr) ;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 497 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = (yyvsp[(1) - (1)].nPtr); ;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 498 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_nop(); ;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 502 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_ARGUMENT_LIST, 1, (yyvsp[(1) - (1)].nPtr)); ;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 503 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 507 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_METHOD_ARGUMENT, 3, ast_nop(), ast_identifier((yyvsp[(1) - (1)].sVal)), ast_nop()); ;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 508 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_METHOD_ARGUMENT, 3, ast_nop(), ast_identifier((yyvsp[(1) - (3)].sVal)), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 509 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_METHOD_ARGUMENT, 3, ast_identifier((yyvsp[(1) - (2)].sVal)), ast_identifier((yyvsp[(2) - (2)].sVal)), ast_nop()); ;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 510 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_METHOD_ARGUMENT, 3, ast_identifier((yyvsp[(1) - (4)].sVal)), ast_identifier((yyvsp[(2) - (4)].sVal)), (yyvsp[(4) - (4)].nPtr)); ;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 514 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_CONSTANTS, 1, (yyvsp[(1) - (1)].nPtr)); ;}
    break;

  case 171:

/* Line 1455 of yacc.c  */
#line 515 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), (yyvsp[(1) - (2)].nPtr)); ;}
    break;

  case 172:

/* Line 1455 of yacc.c  */
#line 519 "compiler/saffire.y"
    { TRACE sfc_validate_constant((yyvsp[(2) - (5)].sVal)); (yyval.nPtr) = ast_opr(T_CONST, 2, ast_identifier((yyvsp[(2) - (5)].sVal)), (yyvsp[(4) - (5)].nPtr)); ;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 523 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_class(global_table->active_class, (yyvsp[(3) - (4)].nPtr)); sfc_fini_class(); ;}
    break;

  case 174:

/* Line 1455 of yacc.c  */
#line 524 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_class(global_table->active_class, ast_nop()); sfc_fini_class(); ;}
    break;

  case 175:

/* Line 1455 of yacc.c  */
#line 528 "compiler/saffire.y"
    { sfc_validate_class_modifiers((yyvsp[(1) - (5)].lVal)); sfc_init_class((yyvsp[(1) - (5)].lVal), (yyvsp[(3) - (5)].sVal)); ;}
    break;

  case 176:

/* Line 1455 of yacc.c  */
#line 529 "compiler/saffire.y"
    { sfc_init_class( 0, (yyvsp[(2) - (4)].sVal)); ;}
    break;

  case 177:

/* Line 1455 of yacc.c  */
#line 533 "compiler/saffire.y"
    { TRACE sfc_validate_class_modifiers((yyvsp[(1) - (7)].lVal)); (yyval.nPtr) = ast_interface((yyvsp[(1) - (7)].lVal), (yyvsp[(3) - (7)].sVal), (yyvsp[(4) - (7)].nPtr), (yyvsp[(6) - (7)].nPtr)); ;}
    break;

  case 178:

/* Line 1455 of yacc.c  */
#line 534 "compiler/saffire.y"
    { TRACE sfc_validate_class_modifiers((yyvsp[(1) - (6)].lVal)); (yyval.nPtr) = ast_interface((yyvsp[(1) - (6)].lVal), (yyvsp[(3) - (6)].sVal), (yyvsp[(4) - (6)].nPtr), ast_nop()); ;}
    break;

  case 179:

/* Line 1455 of yacc.c  */
#line 535 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_interface(0, (yyvsp[(2) - (6)].sVal), (yyvsp[(3) - (6)].nPtr), (yyvsp[(5) - (6)].nPtr)); ;}
    break;

  case 180:

/* Line 1455 of yacc.c  */
#line 536 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_interface(0, (yyvsp[(2) - (5)].sVal), (yyvsp[(3) - (5)].nPtr), ast_nop()); ;}
    break;

  case 181:

/* Line 1455 of yacc.c  */
#line 540 "compiler/saffire.y"
    { TRACE sfc_validate_property_modifiers((yyvsp[(1) - (6)].lVal)); (yyval.nPtr) = ast_opr(T_PROPERTY, 3, ast_numerical((yyvsp[(1) - (6)].lVal)), ast_identifier((yyvsp[(3) - (6)].sVal)), (yyvsp[(5) - (6)].nPtr)); ;}
    break;

  case 182:

/* Line 1455 of yacc.c  */
#line 541 "compiler/saffire.y"
    { TRACE sfc_validate_property_modifiers((yyvsp[(1) - (4)].lVal)); (yyval.nPtr) = ast_opr(T_PROPERTY, 2, ast_numerical((yyvsp[(1) - (4)].lVal)), ast_identifier((yyvsp[(3) - (4)].sVal))); ;}
    break;

  case 183:

/* Line 1455 of yacc.c  */
#line 545 "compiler/saffire.y"
    { TRACE sfc_validate_property_modifiers((yyvsp[(1) - (4)].lVal)); (yyval.nPtr) = ast_opr(T_PROPERTY, 2, ast_numerical((yyvsp[(1) - (4)].lVal)), ast_identifier((yyvsp[(3) - (4)].sVal))); ;}
    break;

  case 184:

/* Line 1455 of yacc.c  */
#line 549 "compiler/saffire.y"
    { TRACE (yyval.lVal) = (yyvsp[(1) - (1)].lVal) ;}
    break;

  case 185:

/* Line 1455 of yacc.c  */
#line 550 "compiler/saffire.y"
    { TRACE sfc_validate_flags((yyval.lVal), (yyvsp[(2) - (2)].lVal)); (yyval.lVal) |= (yyvsp[(2) - (2)].lVal); ;}
    break;

  case 186:

/* Line 1455 of yacc.c  */
#line 555 "compiler/saffire.y"
    { TRACE (yyval.lVal) = MODIFIER_PROTECTED; ;}
    break;

  case 187:

/* Line 1455 of yacc.c  */
#line 556 "compiler/saffire.y"
    { TRACE (yyval.lVal) = MODIFIER_PUBLIC; ;}
    break;

  case 188:

/* Line 1455 of yacc.c  */
#line 557 "compiler/saffire.y"
    { TRACE (yyval.lVal) = MODIFIER_PRIVATE; ;}
    break;

  case 189:

/* Line 1455 of yacc.c  */
#line 558 "compiler/saffire.y"
    { TRACE (yyval.lVal) = MODIFIER_FINAL; ;}
    break;

  case 190:

/* Line 1455 of yacc.c  */
#line 559 "compiler/saffire.y"
    { TRACE (yyval.lVal) = MODIFIER_ABSTRACT; ;}
    break;

  case 191:

/* Line 1455 of yacc.c  */
#line 560 "compiler/saffire.y"
    { TRACE (yyval.lVal) = MODIFIER_STATIC; ;}
    break;

  case 192:

/* Line 1455 of yacc.c  */
#line 561 "compiler/saffire.y"
    { TRACE (yyval.lVal) = MODIFIER_READONLY; ;}
    break;

  case 193:

/* Line 1455 of yacc.c  */
#line 566 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_string((yyvsp[(2) - (2)].sVal)); ;}
    break;

  case 194:

/* Line 1455 of yacc.c  */
#line 567 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_nop(); ;}
    break;

  case 195:

/* Line 1455 of yacc.c  */
#line 572 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_IMPLEMENTS, 1, (yyvsp[(2) - (2)].nPtr)); ;}
    break;

  case 196:

/* Line 1455 of yacc.c  */
#line 573 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_nop(); ;}
    break;

  case 197:

/* Line 1455 of yacc.c  */
#line 578 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_STATEMENTS, 1, ast_string((yyvsp[(1) - (1)].sVal))); ;}
    break;

  case 198:

/* Line 1455 of yacc.c  */
#line 579 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), ast_string((yyvsp[(3) - (3)].sVal))); ;}
    break;

  case 199:

/* Line 1455 of yacc.c  */
#line 591 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_DATA_STRUCTURE, 2, (yyvsp[(1) - (4)].nPtr), (yyvsp[(3) - (4)].nPtr)); ;}
    break;

  case 200:

/* Line 1455 of yacc.c  */
#line 592 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_DATA_STRUCTURE, 2, (yyvsp[(1) - (5)].nPtr), (yyvsp[(3) - (5)].nPtr)); ;}
    break;

  case 201:

/* Line 1455 of yacc.c  */
#line 593 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_DATA_STRUCTURE, 1, (yyvsp[(1) - (3)].nPtr)); ;}
    break;

  case 202:

/* Line 1455 of yacc.c  */
#line 594 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_DATA_STRUCTURE, 2, (yyvsp[(1) - (4)].nPtr), (yyvsp[(3) - (4)].nPtr)); ;}
    break;

  case 203:

/* Line 1455 of yacc.c  */
#line 595 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_DATA_STRUCTURE, 2, (yyvsp[(1) - (5)].nPtr), (yyvsp[(3) - (5)].nPtr)); ;}
    break;

  case 204:

/* Line 1455 of yacc.c  */
#line 596 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_DATA_STRUCTURE, 1, (yyvsp[(1) - (3)].nPtr)); ;}
    break;

  case 205:

/* Line 1455 of yacc.c  */
#line 600 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_DATA_ELEMENTS, 1, (yyvsp[(1) - (1)].nPtr)); ;}
    break;

  case 206:

/* Line 1455 of yacc.c  */
#line 601 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;

  case 207:

/* Line 1455 of yacc.c  */
#line 605 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_opr(T_DATA_ELEMENT, 1, (yyvsp[(1) - (1)].nPtr)); ;}
    break;

  case 208:

/* Line 1455 of yacc.c  */
#line 606 "compiler/saffire.y"
    { TRACE (yyval.nPtr) = ast_add((yyval.nPtr), (yyvsp[(3) - (3)].nPtr)); ;}
    break;



/* Line 1455 of yacc.c  */
#line 3361 "compiler/parser.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 609 "compiler/saffire.y"


   /* Returns the actual name of a token, must be implemented here, because we have no access
      to the yytoknum and yytname otherwise. */
   char *get_token_string(int token) {
        for (int i=0; i!=YYNTOKENS; i++) {
            if (token == yytoknum[i]) {
                return (char *)yytname[i];
            }
        }
        return "<unknown>";
    }

