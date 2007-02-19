/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     tAGO = 258,
     tDAY = 259,
     tDAYZONE = 260,
     tID = 261,
     tMERIDIAN = 262,
     tMINUTE_UNIT = 263,
     tMONTH = 264,
     tMONTH_UNIT = 265,
     tSEC_UNIT = 266,
     tSNUMBER = 267,
     tUNUMBER = 268,
     tZONE = 269,
     tDST = 270
   };
#endif
/* Tokens.  */
#define tAGO 258
#define tDAY 259
#define tDAYZONE 260
#define tID 261
#define tMERIDIAN 262
#define tMINUTE_UNIT 263
#define tMONTH 264
#define tMONTH_UNIT 265
#define tSEC_UNIT 266
#define tSNUMBER 267
#define tUNUMBER 268
#define tZONE 269
#define tDST 270




/* Copy the first part of user declarations.  */
#line 1 "getdate.y"

/*
**  Originally written by Steven M. Bellovin <smb@research.att.com> while
**  at the University of North Carolina at Chapel Hill.  Later tweaked by
**  a couple of people on Usenet.  Completely overhauled by Rich $alz
**  <rsalz@bbn.com> and Jim Berets <jberets@bbn.com> in August, 1990;
**
**  This grammar has 10 shift/reduce conflicts.
**
**  This code is in the public domain and has no copyright.
*/
/*
**  Modified to be a reentrant parser and added multiple language support
**  by Eirik A. Nygaard.
*/
/* SUPPRESS 287 on yaccpar_sccsid *//* Unused static variable */
/* SUPPRESS 288 on yyerrlab *//* Label unused */

#include <sys/types.h>
#include <sys/timeb.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "getdate.h"
#include "languages.h"

/* The code at the top of get_date which figures out the offset of the
   current time zone checks various CPP symbols to see if special
   tricks are need, but defaults to using the gettimeofday system call.
   Include <sys/time.h> if that will be used.  */


/* NOTES on rebuilding getdate.c (particularly for inclusion in CVS
   releases):

   We don't want to mess with all the portability hassles of alloca.
   In particular, most (all?) versions of bison will use alloca in
   their parser.  If bison works on your system (e.g. it should work
   with gcc), then go ahead and use it, but the more general solution
   is to use byacc instead of bison, which should generate a portable
   parser.  I played with adding "#define alloca dont_use_alloca", to
   give an error if the parser generator uses alloca (and thus detect
   unportable getdate.c's), but that seems to cause as many problems
   as it solves.  */

#define yyparse getdate_yyparse
#define yylex getdate_yylex
#define yyerror getdate_yyerror

static int yyparse ();
static int yylex ();
static int yyerror ();


/*
** Date tables and which language they belong to.
*/

typedef struct _LANGTABLE {
	const TABLE *MonthDayTable;
	const TABLE *UnitsTable;
	const TABLE *OtherTable;
	const TABLE *TimezoneTable;
	const TABLE *MilitaryTable;
} LANGTABLE;

LANGTABLE languagetable[LANGUAGE_MAX];
#define lookup_language(lang) (lang > LANGUAGE_LAST) ? NULL : &languagetable[lang]


/*
**  Daylight-savings mode:  on, off, or not yet known.
*/
typedef enum _DSTMODE {
    DSTon, DSToff, DSTmaybe
} DSTMODE;

/*
**  Meridian:  am, pm, or 24-hour style.
*/
typedef enum _MERIDIAN {
    MERam, MERpm, MER24
} MERIDIAN;

/*
**  Global variables.  We could get rid of most of these by using a good
**  union as the yacc stack.  (This routine was originally written before
**  yacc had the %union construct.)  Maybe someday; right now we only use
**  the %union very rarely.
*/

struct yyglobalvars {
	DSTMODE	yyDSTmode;
	time_t	yyDayOrdinal;
	time_t	yyDayNumber;
	int	yyHaveDate;
	int	yyHaveDay;
	int	yyHaveRel;
	int	yyHaveTime;
	int	yyHaveZone;
	time_t	yyTimezone;
	time_t	yyDay;
	time_t	yyHour;
	time_t	yyMinutes;
	time_t	yyMonth;
	time_t	yySeconds;
	time_t	yyYear;
	MERIDIAN	yyMeridian;
	time_t	yyRelMonth;
	time_t	yyRelSeconds;
};



/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 126 "getdate.y"
{
    time_t		Number;
    enum _MERIDIAN	Meridian;
}
/* Line 193 of yacc.c.  */
#line 249 "getdate.tab.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 216 of yacc.c.  */
#line 262 "getdate.tab.c"

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
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
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
  yytype_int16 yyss;
  YYSTYPE yyvs;
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
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   53

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  20
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  12
/* YYNRULES -- Number of rules.  */
#define YYNRULES  44
/* YYNRULES -- Number of states.  */
#define YYNSTATES  63

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   270

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    18,     2,    16,    19,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    17,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint8 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    15,    17,
      19,    21,    33,    36,    41,    46,    53,    60,    62,    64,
      67,    69,    72,    75,    79,    85,    89,    93,    96,   101,
     104,   108,   111,   113,   116,   119,   121,   124,   127,   129,
     132,   135,   137,   139,   140
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      21,     0,    -1,    -1,    21,    22,    -1,    24,    -1,    25,
      -1,    27,    -1,    26,    -1,    28,    -1,    23,    -1,    30,
      -1,    13,    16,    13,    16,    13,    16,    13,    16,    13,
      16,    13,    -1,    13,     7,    -1,    13,    17,    13,    31,
      -1,    13,    17,    13,    12,    -1,    13,    17,    13,    17,
      13,    31,    -1,    13,    17,    13,    17,    13,    12,    -1,
      14,    -1,     5,    -1,    14,    15,    -1,     4,    -1,     4,
      18,    -1,    13,     4,    -1,    13,    19,    13,    -1,    13,
      19,    13,    19,    13,    -1,    13,    12,    12,    -1,    13,
       9,    12,    -1,     9,    13,    -1,     9,    13,    18,    13,
      -1,    13,     9,    -1,    13,     9,    13,    -1,    29,     3,
      -1,    29,    -1,    13,     8,    -1,    12,     8,    -1,     8,
      -1,    12,    11,    -1,    13,    11,    -1,    11,    -1,    12,
      10,    -1,    13,    10,    -1,    10,    -1,    13,    -1,    -1,
       7,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   140,   140,   141,   144,   147,   150,   153,   156,   159,
     164,   167,   180,   186,   192,   199,   205,   215,   219,   224,
     230,   234,   238,   244,   248,   259,   265,   271,   275,   280,
     284,   291,   295,   298,   301,   304,   307,   310,   313,   316,
     319,   322,   327,   354,   357
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "tAGO", "tDAY", "tDAYZONE", "tID",
  "tMERIDIAN", "tMINUTE_UNIT", "tMONTH", "tMONTH_UNIT", "tSEC_UNIT",
  "tSNUMBER", "tUNUMBER", "tZONE", "tDST", "'.'", "':'", "','", "'/'",
  "$accept", "spec", "item", "cvsstamp", "time", "zone", "day", "date",
  "rel", "relunit", "number", "o_merid", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,    46,    58,    44,    47
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    20,    21,    21,    22,    22,    22,    22,    22,    22,
      22,    23,    24,    24,    24,    24,    24,    25,    25,    25,
      26,    26,    26,    27,    27,    27,    27,    27,    27,    27,
      27,    28,    28,    29,    29,    29,    29,    29,    29,    29,
      29,    29,    30,    31,    31
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     1,
       1,    11,     2,     4,     4,     6,     6,     1,     1,     2,
       1,     2,     2,     3,     5,     3,     3,     2,     4,     2,
       3,     2,     1,     2,     2,     1,     2,     2,     1,     2,
       2,     1,     1,     0,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,    20,    18,    35,     0,    41,    38,     0,
      42,    17,     3,     9,     4,     5,     7,     6,     8,    32,
      10,    21,    27,    34,    39,    36,    22,    12,    33,    29,
      40,    37,     0,     0,     0,     0,    19,    31,     0,    26,
      30,    25,     0,    43,    23,    28,     0,    44,    14,     0,
      13,     0,     0,    43,    24,     0,    16,    15,     0,     0,
       0,     0,    11
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    50
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -16
static const yytype_int8 yypact[] =
{
     -16,    14,   -16,    -9,   -16,   -16,    -3,   -16,   -16,    25,
      -4,     2,   -16,   -16,   -16,   -16,   -16,   -16,   -16,    17,
     -16,   -16,    12,   -16,   -16,   -16,   -16,   -16,   -16,   -11,
     -16,   -16,    19,    24,    26,    27,   -16,   -16,    28,   -16,
     -16,   -16,    16,     4,    23,   -16,    30,   -16,   -16,    31,
     -16,    32,    33,    22,   -16,    34,   -16,   -16,    35,    37,
      36,    40,   -16
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -16,   -16,   -16,   -16,   -16,   -16,   -16,   -16,   -16,   -16,
     -16,   -15
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
      26,    39,    40,    27,    28,    29,    30,    31,    32,    21,
      22,    47,    33,    34,     2,    35,    48,    36,     3,     4,
      37,    49,     5,     6,     7,     8,     9,    10,    11,    47,
      38,    41,    46,    23,    56,    24,    25,    42,    57,    43,
      44,    45,    51,    52,    53,    54,     0,    58,     0,    55,
      60,    59,    61,    62
};

static const yytype_int8 yycheck[] =
{
       4,    12,    13,     7,     8,     9,    10,    11,    12,    18,
      13,     7,    16,    17,     0,    19,    12,    15,     4,     5,
       3,    17,     8,     9,    10,    11,    12,    13,    14,     7,
      18,    12,    16,     8,    12,    10,    11,    13,    53,    13,
      13,    13,    19,    13,    13,    13,    -1,    13,    -1,    16,
      13,    16,    16,    13
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    21,     0,     4,     5,     8,     9,    10,    11,    12,
      13,    14,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    18,    13,     8,    10,    11,     4,     7,     8,     9,
      10,    11,    12,    16,    17,    19,    15,     3,    18,    12,
      13,    12,    13,    13,    13,    13,    16,     7,    12,    17,
      31,    19,    13,    13,    13,    16,    12,    31,    13,    16,
      13,    16,    13
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
      yyerror (input, lt, yyvars, YY_("syntax error: cannot back up")); \
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
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, input, lt)
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
		  Type, Value, input, lt, yyvars); \
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
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, char **input, LANGTABLE *lt, struct yyglobalvars *yyvars)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep, input, lt, yyvars)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    char **input;
    LANGTABLE *lt;
    struct yyglobalvars *yyvars;
#endif
{
  if (!yyvaluep)
    return;
  YYUSE (input);
  YYUSE (lt);
  YYUSE (yyvars);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, char **input, LANGTABLE *lt, struct yyglobalvars *yyvars)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep, input, lt, yyvars)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
    char **input;
    LANGTABLE *lt;
    struct yyglobalvars *yyvars;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, input, lt, yyvars);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
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
yy_reduce_print (YYSTYPE *yyvsp, int yyrule, char **input, LANGTABLE *lt, struct yyglobalvars *yyvars)
#else
static void
yy_reduce_print (yyvsp, yyrule, input, lt, yyvars)
    YYSTYPE *yyvsp;
    int yyrule;
    char **input;
    LANGTABLE *lt;
    struct yyglobalvars *yyvars;
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
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       , input, lt, yyvars);
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule, input, lt, yyvars); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, char **input, LANGTABLE *lt, struct yyglobalvars *yyvars)
#else
static void
yydestruct (yymsg, yytype, yyvaluep, input, lt, yyvars)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
    char **input;
    LANGTABLE *lt;
    struct yyglobalvars *yyvars;
#endif
{
  YYUSE (yyvaluep);
  YYUSE (input);
  YYUSE (lt);
  YYUSE (yyvars);

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
int yyparse (char **input, LANGTABLE *lt, struct yyglobalvars *yyvars);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

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
yyparse (char **input, LANGTABLE *lt, struct yyglobalvars *yyvars)
#else
int
yyparse (input, lt, yyvars)
    char **input;
    LANGTABLE *lt;
    struct yyglobalvars *yyvars;
#endif
#endif
{
  /* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

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
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
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

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
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
        case 4:
#line 144 "getdate.y"
    {
	    yyvars->yyHaveTime++;
	;}
    break;

  case 5:
#line 147 "getdate.y"
    {
	    yyvars->yyHaveZone++;
	;}
    break;

  case 6:
#line 150 "getdate.y"
    {
	    yyvars->yyHaveDate++;
	;}
    break;

  case 7:
#line 153 "getdate.y"
    {
	    yyvars->yyHaveDay++;
	;}
    break;

  case 8:
#line 156 "getdate.y"
    {
	    yyvars->yyHaveRel++;
	;}
    break;

  case 9:
#line 159 "getdate.y"
    {
	    yyvars->yyHaveTime++;
	    yyvars->yyHaveDate++;
	    yyvars->yyHaveZone++;
	;}
    break;

  case 11:
#line 167 "getdate.y"
    {
	    yyvars->yyYear = (yyvsp[(1) - (11)].Number);
	    if (yyvars->yyYear < 100) yyvars->yyYear += 1900;
	    yyvars->yyMonth = (yyvsp[(3) - (11)].Number);
	    yyvars->yyDay = (yyvsp[(5) - (11)].Number);
	    yyvars->yyHour = (yyvsp[(7) - (11)].Number);
	    yyvars->yyMinutes = (yyvsp[(9) - (11)].Number);
	    yyvars->yySeconds = (yyvsp[(11) - (11)].Number);
	    yyvars->yyDSTmode = DSToff;
	    yyvars->yyTimezone = 0;
	;}
    break;

  case 12:
#line 180 "getdate.y"
    {
	    yyvars->yyHour = (yyvsp[(1) - (2)].Number);
	    yyvars->yyMinutes = 0;
	    yyvars->yySeconds = 0;
	    yyvars->yyMeridian = (yyvsp[(2) - (2)].Meridian);
	;}
    break;

  case 13:
#line 186 "getdate.y"
    {
	    yyvars->yyHour = (yyvsp[(1) - (4)].Number);
	    yyvars->yyMinutes = (yyvsp[(3) - (4)].Number);
	    yyvars->yySeconds = 0;
	    yyvars->yyMeridian = (yyvsp[(4) - (4)].Meridian);
	;}
    break;

  case 14:
#line 192 "getdate.y"
    {
	    yyvars->yyHour = (yyvsp[(1) - (4)].Number);
	    yyvars->yyMinutes = (yyvsp[(3) - (4)].Number);
	    yyvars->yyMeridian = MER24;
	    yyvars->yyDSTmode = DSToff;
	    yyvars->yyTimezone = - ((yyvsp[(4) - (4)].Number) % 100 + ((yyvsp[(4) - (4)].Number) / 100) * 60);
	;}
    break;

  case 15:
#line 199 "getdate.y"
    {
	    yyvars->yyHour = (yyvsp[(1) - (6)].Number);
	    yyvars->yyMinutes = (yyvsp[(3) - (6)].Number);
	    yyvars->yySeconds = (yyvsp[(5) - (6)].Number);
	    yyvars->yyMeridian = (yyvsp[(6) - (6)].Meridian);
	;}
    break;

  case 16:
#line 205 "getdate.y"
    {
	    yyvars->yyHour = (yyvsp[(1) - (6)].Number);
	    yyvars->yyMinutes = (yyvsp[(3) - (6)].Number);
	    yyvars->yySeconds = (yyvsp[(5) - (6)].Number);
	    yyvars->yyMeridian = MER24;
	    yyvars->yyDSTmode = DSToff;
	    yyvars->yyTimezone = - ((yyvsp[(6) - (6)].Number) % 100 + ((yyvsp[(6) - (6)].Number) / 100) * 60);
	;}
    break;

  case 17:
#line 215 "getdate.y"
    {
	    yyvars->yyTimezone = (yyvsp[(1) - (1)].Number);
	    yyvars->yyDSTmode = DSToff;
	;}
    break;

  case 18:
#line 219 "getdate.y"
    {
	    yyvars->yyTimezone = (yyvsp[(1) - (1)].Number);
	    yyvars->yyDSTmode = DSTon;
	;}
    break;

  case 19:
#line 224 "getdate.y"
    {
	    yyvars->yyTimezone = (yyvsp[(1) - (2)].Number);
	    yyvars->yyDSTmode = DSTon;
	;}
    break;

  case 20:
#line 230 "getdate.y"
    {
	    yyvars->yyDayOrdinal = 1;
	    yyvars->yyDayNumber = (yyvsp[(1) - (1)].Number);
	;}
    break;

  case 21:
#line 234 "getdate.y"
    {
	    yyvars->yyDayOrdinal = 1;
	    yyvars->yyDayNumber = (yyvsp[(1) - (2)].Number);
	;}
    break;

  case 22:
#line 238 "getdate.y"
    {
	    yyvars->yyDayOrdinal = (yyvsp[(1) - (2)].Number);
	    yyvars->yyDayNumber = (yyvsp[(2) - (2)].Number);
	;}
    break;

  case 23:
#line 244 "getdate.y"
    {
	    yyvars->yyMonth = (yyvsp[(1) - (3)].Number);
	    yyvars->yyDay = (yyvsp[(3) - (3)].Number);
	;}
    break;

  case 24:
#line 248 "getdate.y"
    {
	    if ((yyvsp[(1) - (5)].Number) >= 100) {
		yyvars->yyYear = (yyvsp[(1) - (5)].Number);
		yyvars->yyMonth = (yyvsp[(3) - (5)].Number);
		yyvars->yyDay = (yyvsp[(5) - (5)].Number);
	    } else {
		yyvars->yyMonth = (yyvsp[(1) - (5)].Number);
		yyvars->yyDay = (yyvsp[(3) - (5)].Number);
		yyvars->yyYear = (yyvsp[(5) - (5)].Number);
	    }
	;}
    break;

  case 25:
#line 259 "getdate.y"
    {
	    /* ISO 8601 format.  yyyy-mm-dd.  */
	    yyvars->yyYear = (yyvsp[(1) - (3)].Number);
	    yyvars->yyMonth = -(yyvsp[(2) - (3)].Number);
	    yyvars->yyDay = -(yyvsp[(3) - (3)].Number);
	;}
    break;

  case 26:
#line 265 "getdate.y"
    {
	    /* e.g. 17-JUN-1992.  */
	    yyvars->yyDay = (yyvsp[(1) - (3)].Number);
	    yyvars->yyMonth = (yyvsp[(2) - (3)].Number);
	    yyvars->yyYear = -(yyvsp[(3) - (3)].Number);
	;}
    break;

  case 27:
#line 271 "getdate.y"
    {
	    yyvars->yyMonth = (yyvsp[(1) - (2)].Number);
	    yyvars->yyDay = (yyvsp[(2) - (2)].Number);
	;}
    break;

  case 28:
#line 275 "getdate.y"
    {
	    yyvars->yyMonth = (yyvsp[(1) - (4)].Number);
	    yyvars->yyDay = (yyvsp[(2) - (4)].Number);
	    yyvars->yyYear = (yyvsp[(4) - (4)].Number);
	;}
    break;

  case 29:
#line 280 "getdate.y"
    {
	    yyvars->yyMonth = (yyvsp[(2) - (2)].Number);
	    yyvars->yyDay = (yyvsp[(1) - (2)].Number);
	;}
    break;

  case 30:
#line 284 "getdate.y"
    {
	    yyvars->yyMonth = (yyvsp[(2) - (3)].Number);
	    yyvars->yyDay = (yyvsp[(1) - (3)].Number);
	    yyvars->yyYear = (yyvsp[(3) - (3)].Number);
	;}
    break;

  case 31:
#line 291 "getdate.y"
    {
	    yyvars->yyRelSeconds = -yyvars->yyRelSeconds;
	    yyvars->yyRelMonth = -yyvars->yyRelMonth;
	;}
    break;

  case 33:
#line 298 "getdate.y"
    {
	    yyvars->yyRelSeconds += (yyvsp[(1) - (2)].Number) * (yyvsp[(2) - (2)].Number) * 60L;
	;}
    break;

  case 34:
#line 301 "getdate.y"
    {
	    yyvars->yyRelSeconds += (yyvsp[(1) - (2)].Number) * (yyvsp[(2) - (2)].Number) * 60L;
	;}
    break;

  case 35:
#line 304 "getdate.y"
    {
	    yyvars->yyRelSeconds += (yyvsp[(1) - (1)].Number) * 60L;
	;}
    break;

  case 36:
#line 307 "getdate.y"
    {
	    yyvars->yyRelSeconds += (yyvsp[(1) - (2)].Number);
	;}
    break;

  case 37:
#line 310 "getdate.y"
    {
	    yyvars->yyRelSeconds += (yyvsp[(1) - (2)].Number);
	;}
    break;

  case 38:
#line 313 "getdate.y"
    {
	    yyvars->yyRelSeconds++;
	;}
    break;

  case 39:
#line 316 "getdate.y"
    {
	    yyvars->yyRelMonth += (yyvsp[(1) - (2)].Number) * (yyvsp[(2) - (2)].Number);
	;}
    break;

  case 40:
#line 319 "getdate.y"
    {
	    yyvars->yyRelMonth += (yyvsp[(1) - (2)].Number) * (yyvsp[(2) - (2)].Number);
	;}
    break;

  case 41:
#line 322 "getdate.y"
    {
	    yyvars->yyRelMonth += (yyvsp[(1) - (1)].Number);
	;}
    break;

  case 42:
#line 327 "getdate.y"
    {
	    if (yyvars->yyHaveTime && yyvars->yyHaveDate && !yyvars->yyHaveRel)
		yyvars->yyYear = (yyvsp[(1) - (1)].Number);
	    else {
		if((yyvsp[(1) - (1)].Number)>10000) {
		    yyvars->yyHaveDate++;
		    yyvars->yyDay= ((yyvsp[(1) - (1)].Number))%100;
		    yyvars->yyMonth= ((yyvsp[(1) - (1)].Number)/100)%100;
		    yyvars->yyYear = (yyvsp[(1) - (1)].Number)/10000;
		}
		else {
		    yyvars->yyHaveTime++;
		    if ((yyvsp[(1) - (1)].Number) < 100) {
			yyvars->yyHour = (yyvsp[(1) - (1)].Number);
			yyvars->yyMinutes = 0;
		    }
		    else {
		    	yyvars->yyHour = (yyvsp[(1) - (1)].Number) / 100;
		    	yyvars->yyMinutes = (yyvsp[(1) - (1)].Number) % 100;
		    }
		    yyvars->yySeconds = 0;
		    yyvars->yyMeridian = MER24;
	        }
	    }
	;}
    break;

  case 43:
#line 354 "getdate.y"
    {
	    (yyval.Meridian) = MER24;
	;}
    break;

  case 44:
#line 357 "getdate.y"
    {
	    (yyval.Meridian) = (yyvsp[(1) - (1)].Meridian);
	;}
    break;


/* Line 1267 of yacc.c.  */
#line 1878 "getdate.tab.c"
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
      yyerror (input, lt, yyvars, YY_("syntax error"));
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
	    yyerror (input, lt, yyvars, yymsg);
	  }
	else
	  {
	    yyerror (input, lt, yyvars, YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
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
		      yytoken, &yylval, input, lt, yyvars);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
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
		  yystos[yystate], yyvsp, input, lt, yyvars);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

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

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (input, lt, yyvars, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval, input, lt, yyvars);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp, input, lt, yyvars);
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


#line 362 "getdate.y"



/* ARGSUSED */
static int
yyerror(s)
    char	*s;
{
  return 0;
}


static time_t
ToSeconds(Hours, Minutes, Seconds, Meridian)
    time_t	Hours;
    time_t	Minutes;
    time_t	Seconds;
    MERIDIAN	Meridian;
{
    if (Minutes < 0 || Minutes > 59 || Seconds < 0 || Seconds > 59)
	return -1;
    switch (Meridian) {
    case MER24:
	if (Hours < 0 || Hours > 23)
	    return -1;
	return (Hours * 60L + Minutes) * 60L + Seconds;
    case MERam:
	if (Hours < 1 || Hours > 12)
	    return -1;
	if (Hours == 12)
	    Hours = 0;
	return (Hours * 60L + Minutes) * 60L + Seconds;
    case MERpm:
	if (Hours < 1 || Hours > 12)
	    return -1;
	if (Hours == 12)
	    Hours = 0;
	return ((Hours + 12) * 60L + Minutes) * 60L + Seconds;
    default:
	abort ();
    }
    /* NOTREACHED */
}


/* Year is either
   * A negative number, which means to use its absolute value (why?)
   * A number from 0 to 99, which means a year from 1900 to 1999, or
   * The actual year (>=100).  */
static time_t
Convert(Month, Day, Year, Hours, Minutes, Seconds, Meridian, DSTmode, yyvars)
    time_t	Month;
    time_t	Day;
    time_t	Year;
    time_t	Hours;
    time_t	Minutes;
    time_t	Seconds;
    MERIDIAN	Meridian;
    DSTMODE	DSTmode;
    struct yyglobalvars *yyvars;
{
    static int DaysInMonth[12] = {
	31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };
    time_t	tod;
    time_t	Julian;
    int		i;

    if (Year < 0)
	Year = -Year;
    if (Year < 69)
	Year += 2000;
    else if (Year < 100)
	Year += 1900;
    DaysInMonth[1] = Year % 4 == 0 && (Year % 100 != 0 || Year % 400 == 0)
		    ? 29 : 28;
    /* Checking for 2038 bogusly assumes that time_t is 32 bits.  But
       I'm too lazy to try to check for time_t overflow in another way.  */
    if (Year < EPOCH || Year > 2038
     || Month < 1 || Month > 12
     /* Lint fluff:  "conversion from long may lose accuracy" */
     || Day < 1 || Day > DaysInMonth[(int)--Month])
	/* FIXME:
	 * It would be nice to set a global error string here.
	 * "February 30 is not a valid date" is much more informative than
	 * "Can't parse date/time: 100 months" when the user input was
	 * "100 months" and addition resolved that to February 30, for
	 * example.  See rcs2-7 in src/sanity.sh for more. */
	return -1;

    for (Julian = Day - 1, i = 0; i < Month; i++)
	Julian += DaysInMonth[i];
    for (i = EPOCH; i < Year; i++)
	Julian += 365 + (i % 4 == 0);
    Julian *= SECSPERDAY;
    Julian += yyvars->yyTimezone * 60L;
    if ((tod = ToSeconds(Hours, Minutes, Seconds, Meridian)) < 0)
	return -1;
    Julian += tod;
    if (DSTmode == DSTon
     || (DSTmode == DSTmaybe && localtime(&Julian)->tm_isdst))
	Julian -= 60 * 60;
    return Julian;
}


static time_t
DSTcorrect(Start, Future)
    time_t	Start;
    time_t	Future;
{
    time_t	StartDay;
    time_t	FutureDay;

    StartDay = (localtime(&Start)->tm_hour + 1) % 24;
    FutureDay = (localtime(&Future)->tm_hour + 1) % 24;
    return (Future - Start) + (StartDay - FutureDay) * 60L * 60L;
}


static time_t
RelativeDate(Start, DayOrdinal, DayNumber)
    time_t	Start;
    time_t	DayOrdinal;
    time_t	DayNumber;
{
    struct tm	*tm;
    time_t	now;

    now = Start;
    tm = localtime(&now);
    now += SECSPERDAY * ((DayNumber - tm->tm_wday + 7) % 7);
    now += 7 * SECSPERDAY * (DayOrdinal <= 0 ? DayOrdinal : DayOrdinal - 1);
    return DSTcorrect(Start, now);
}


static time_t
RelativeMonth(Start, RelMonth, yyvars)
    time_t	Start;
    time_t	RelMonth;
    struct yyglobalvars *yyvars;
{
    struct tm	*tm;
    time_t	Month;
    time_t	Year;

    if (RelMonth == 0)
	return 0;
    tm = localtime(&Start);
    Month = 12 * (tm->tm_year + 1900) + tm->tm_mon + RelMonth;
    Year = Month / 12;
    Month = Month % 12 + 1;
    return DSTcorrect(Start,
	    Convert(Month, (time_t)tm->tm_mday, Year,
		(time_t)tm->tm_hour, (time_t)tm->tm_min, (time_t)tm->tm_sec,
		MER24, DSTmaybe, yyvars));
}


static int
LookupWord(buff, lt, yylval)
    char		*buff;
    LANGTABLE		*lt;
    YYSTYPE		*yylval;
{
    register char	*p;
    register char	*q;
    register const TABLE	*tp;
    int			i;
    int			abbrev;

    /* Make it lowercase. */
    for (p = buff; *p; p++)
	if (isupper(*p))
	    *p = tolower(*p);

    if (strcmp(buff, "am") == 0 || strcmp(buff, "a.m.") == 0) {
	yylval->Meridian = MERam;
	return tMERIDIAN;
    }
    if (strcmp(buff, "pm") == 0 || strcmp(buff, "p.m.") == 0) {
	yylval->Meridian = MERpm;
	return tMERIDIAN;
    }

    /* See if we have an abbreviation for a month. */
    if (strlen(buff) == 3)
	abbrev = 1;
    else if (strlen(buff) == 4 && buff[3] == '.') {
	abbrev = 1;
	buff[3] = '\0';
    }
    else
	abbrev = 0;

    for (tp = lt->MonthDayTable; tp->name; tp++) {
	if (abbrev) {
	    if (strncmp(buff, tp->name, 3) == 0) {
		yylval->Number = tp->value;
		return tp->type;
	    }
	}
	else if (strcmp(buff, tp->name) == 0) {
	    yylval->Number = tp->value;
	    return tp->type;
	}
    }

    for (tp = lt->TimezoneTable; tp->name; tp++)
	if (strcmp(buff, tp->name) == 0) {
	    yylval->Number = tp->value;
	    return tp->type;
	}

    if (strcmp(buff, "dst") == 0) 
	return tDST;

    for (tp = lt->UnitsTable; tp->name; tp++)
	if (strcmp(buff, tp->name) == 0) {
	    yylval->Number = tp->value;
	    return tp->type;
	}

    /* Strip off any plural and try the units table again. */
    /* XXX: Support more than just enligsh plural. */
    i = strlen(buff) - 1;
    if (buff[i] == 's') {
	buff[i] = '\0';
	for (tp = lt->UnitsTable; tp->name; tp++)
	    if (strcmp(buff, tp->name) == 0) {
		yylval->Number = tp->value;
		return tp->type;
	    }
	buff[i] = 's';		/* Put back for "this" in OtherTable. */
    }

    for (tp = lt->OtherTable; tp->name; tp++)
	if (strcmp(buff, tp->name) == 0) {
	    yylval->Number = tp->value;
	    return tp->type;
	}

    /* Military timezones. */
    if (buff[1] == '\0' && isalpha(*buff)) {
	for (tp = lt->MilitaryTable; tp->name; tp++)
	    if (strcmp(buff, tp->name) == 0) {
		yylval->Number = tp->value;
		return tp->type;
	    }
    }

    /* Drop out any periods and try the timezone table again. */
    for (i = 0, p = q = buff; *q; q++)
	if (*q != '.')
	    *p++ = *q;
	else
	    i++;
    *p = '\0';
    if (i)
	for (tp = lt->TimezoneTable; tp->name; tp++)
	    if (strcmp(buff, tp->name) == 0) {
		yylval->Number = tp->value;
		return tp->type;
	    }

    return tID;
}


static int
yylex(YYSTYPE *yylval, char **input, LANGTABLE *lt)
{
    register char	c;
    register char	*p;
    char		buff[20];
    int			Count;
    int			sign;
    char		*yyInput;

    yyInput = *input;
    for ( ; ; ) {
	while (isspace(*yyInput))
	    yyInput++;

	if (isdigit(c = *yyInput) || c == '-' || c == '+') {
	    if (c == '-' || c == '+') {
		sign = c == '-' ? -1 : 1;
		if (!isdigit(*++yyInput))
		    /* skip the '-' sign */
		    continue;
	    }
	    else
		sign = 0;
	    for (yylval->Number = 0; isdigit(c = *yyInput++); )
		yylval->Number = 10 * yylval->Number + c - '0';
	    yyInput--;
	    if (sign < 0)
		yylval->Number = -yylval->Number;
            *input = yyInput;
	    return sign ? tSNUMBER : tUNUMBER;
	}
	if (isalpha(c)) {
	    for (p = buff; isalpha(c = *yyInput++) || c == '.'; )
		if (p < &buff[sizeof buff - 1])
		    *p++ = c;
	    *p = '\0';
	    yyInput--;
            *input = yyInput;
	    return LookupWord(buff, lt, yylval);
	}
	if (c != '(') {
            *input = yyInput;
	    return *yyInput++;
	}
	Count = 0;
	do {
	    c = *yyInput++;
	    if (c == '\0') {
                *input = yyInput;
		return c;
	    }
	    if (c == '(') {
		Count++;
	    }
	    else if (c == ')') {
		Count--;
	    }
	} while (Count > 0);
    }
}

#define TM_YEAR_ORIGIN 1900

/* Yield A - B, measured in seconds.  */
static long
difftm (a, b)
     struct tm *a, *b;
{
  int ay = a->tm_year + (TM_YEAR_ORIGIN - 1);
  int by = b->tm_year + (TM_YEAR_ORIGIN - 1);
  int days = (
	      /* difference in day of year */
	      a->tm_yday - b->tm_yday
	      /* + intervening leap days */
	      +  ((ay >> 2) - (by >> 2))
	      -  (ay/100 - by/100)
	      +  ((ay/100 >> 2) - (by/100 >> 2))
	      /* + difference in years * 365 */
	      +  (long)(ay-by) * 365
	      );
  return (60*(60*(24*days + (a->tm_hour - b->tm_hour))
	      + (a->tm_min - b->tm_min))
	  + (a->tm_sec - b->tm_sec));
}

static time_t
_get_date(p, now, language)
    char		*p;
    struct timeb	*now;
    int			language;
{
    struct tm		*tm, gmt;
    struct timeb	ftz;
    time_t		Start;
    time_t		tod;
    time_t		nowtime;
    LANGTABLE		*langtbl;
    struct yyglobalvars yyvars;

    langtbl = lookup_language(language);
    if (langtbl == NULL)
	    return -1;

    if (now == NULL) {
	struct tm *gmt_ptr;

        now = &ftz;
	(void)time (&nowtime);

	gmt_ptr = gmtime (&nowtime);
	if (gmt_ptr != NULL)
	{
	    /* Make a copy, in case localtime modifies *tm (I think
	       that comment now applies to *gmt_ptr, but I am too
	       lazy to dig into how gmtime and locatime allocate the
	       structures they return pointers to).  */
	    gmt = *gmt_ptr;
	}

	if (! (tm = localtime (&nowtime)))
	    return -1;

	if (gmt_ptr != NULL)
	    ftz.timezone = difftm (&gmt, tm) / 60;
	else
	    /* We are on a system like VMS, where the system clock is
	       in local time and the system has no concept of timezones.
	       Hopefully we can fake this out (for the case in which the
	       user specifies no timezone) by just saying the timezone
	       is zero.  */
	    ftz.timezone = 0;

	if(tm->tm_isdst)
	    ftz.timezone += 60;
    }
    else
    {
	nowtime = now->time;
    }

    tm = localtime(&nowtime);
    yyvars.yyYear = tm->tm_year + 1900;
    yyvars.yyMonth = tm->tm_mon + 1;
    yyvars.yyDay = tm->tm_mday;
    yyvars.yyTimezone = now->timezone;
    yyvars.yyDSTmode = DSTmaybe;
    yyvars.yyHour = 0;
    yyvars.yyMinutes = 0;
    yyvars.yySeconds = 0;
    yyvars.yyMeridian = MER24;
    yyvars.yyRelSeconds = 0;
    yyvars.yyRelMonth = 0;
    yyvars.yyHaveDate = 0;
    yyvars.yyHaveDay = 0;
    yyvars.yyHaveRel = 0;
    yyvars.yyHaveTime = 0;
    yyvars.yyHaveZone = 0;

    if (yyparse(&p, langtbl, &yyvars)
     || yyvars.yyHaveTime > 1 || yyvars.yyHaveZone > 1 || yyvars.yyHaveDate > 1 || yyvars.yyHaveDay > 1)
	return -1;

    if (yyvars.yyHaveDate || yyvars.yyHaveTime || yyvars.yyHaveDay) {
	Start = Convert(yyvars.yyMonth, yyvars.yyDay, yyvars.yyYear, yyvars.yyHour, yyvars.yyMinutes,
	                yyvars.yySeconds, yyvars.yyMeridian, yyvars.yyDSTmode, &yyvars);
	if (Start < 0)
	    return -1;
    }
    else {
	Start = nowtime;
	if (!yyvars.yyHaveRel)
	    Start -= ((tm->tm_hour * 60L + tm->tm_min) * 60L) + tm->tm_sec;
    }

    Start += yyvars.yyRelSeconds;
    Start += RelativeMonth(Start, yyvars.yyRelMonth, &yyvars);

    if (yyvars.yyHaveDay && !yyvars.yyHaveDate) {
	tod = RelativeDate(Start, yyvars.yyDayOrdinal, yyvars.yyDayNumber, &yyvars);
	Start += tod;
    }

    /* Have to do *something* with a legitimate -1 so it's distinguishable
     * from the error return value.  (Alternately could set errno on error.) */
    return Start == -1 ? 0 : Start;
}

time_t
get_date(char *timebuf, struct timeb *tb, int lang, struct timerange *tr)
{
	char *buf, *p;
	time_t starttime, endtime;

	buf = malloc(strlen(timebuf) + 1);
	strcpy(buf, timebuf);

	p = index(buf, '-');
	if (p == NULL || p >= timebuf + strlen(timebuf) - 1) {
		starttime = _get_date(timebuf, tb, lang);
		if (tr != NULL)
			tr->start = tr->end = starttime;
		free(buf);
		return tr->start;
	}
	*p = '\0';
	p++;

	starttime = _get_date(buf, tb, lang);
	endtime = _get_date(p, tb, lang);
	free(buf);
	if (starttime == -1 || endtime == -1)
		return -1;
	
	if (tr != NULL) {
		tr->start = starttime;
		tr->end = endtime;
	}

	return endtime - starttime;
}

/* XXX: Remove this */
void
init_getdate(void)
{

#define add_language_item(suffix, name) do {\
	languagetable[__CONCAT(LANGUAGE_, suffix)].name = __CONCAT(name, suffix);\
} while(0)
#define add_language(suffix) do {\
		add_language_item(suffix, MonthDayTable); \
		add_language_item(suffix, UnitsTable); \
		add_language_item(suffix, OtherTable); \
		add_language_item(suffix, TimezoneTable); \
		add_language_item(suffix, MilitaryTable); \
	} while (0)

	add_language(EN);
	add_language(NO);

#undef add_language
#undef add_language_item
}


#if	defined(TEST)

/* ARGSUSED */
int
main(ac, av)
    int		ac;
    char	*av[];
{
    char	buff[128];
    time_t	d;
    int		language;
    struct timerange tr;

    init_getdate();
    printf("Enter language to use: ");
    fgets(buff, sizeof(buff), stdin);
    language = atoi(buff);
    fflush(stdout);
    printf("Enter date, or blank line to exit. Separate dates with '-'\n\t> ");
    fflush(stdout);
    while (fgets(buff, sizeof(buff), stdin) && buff[0]) {
	d = get_date(buff, NULL, language, &tr);
	if (d == -1) {
	    printf("Bad format - couldn't convert.\n");
	} else {
	    printf("> %s", ctime(&tr.start));
	    printf("> %s", ctime(&tr.end));
	    printf("%s", ctime(&d));
	}

	printf("\t> ");
	fflush(stdout);
    }
    return(0);
}
#endif	/* defined(TEST) */

