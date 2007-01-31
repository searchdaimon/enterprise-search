/* A Bison parser, made by GNU Bison 2.1.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005 Free Software Foundation, Inc.

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

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     WORD = 258,
     TAG_START = 259,
     TAG_STOPP = 260,
     TAG_ENDTAG_STOPP = 261,
     ENDTAG_START = 262,
     ENDTAG_STOPP = 263,
     ATTR = 264,
     EQUALS = 265,
     TEXTFIELD = 266
   };
#endif
/* Tokens.  */
#define WORD 258
#define TAG_START 259
#define TAG_STOPP 260
#define TAG_ENDTAG_STOPP 261
#define ENDTAG_START 262
#define ENDTAG_STOPP 263
#define ATTR 264
#define EQUALS 265
#define TEXTFIELD 266




/* Copy the first part of user declarations.  */
#line 1 "summary.yacc"

// (C) Copyright Boitho 2005, Magnus Galåen (magnusga@idi.ntnu.no)

/******************************************
changelog:
Runarb 13. des 2005
I steden for å bruke globale verider for title, body, metakeyw, metadesc og *current_buffer her jeg 
laget en "struct bufferformat" med disse i. generate_summary oppreter nå en buffers av denne typen 
og sender den med yyparse.

Hva den heter i yyparse defineres med "#define YYPARSE_PARAM buffers"

Dette får å gjøre honteringen av buffere threadsafe. Uten dette vil man ofte få segfeil i buffer_exit når man 
"kjørerfree( b.data );"

*******************************************/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

//navn på en parameter  yyparse skal ta inn http://dinosaur.compilertools.net/bison/bison_7.html#SEC65
#define YYPARSE_PARAM buffers

#include "summary_common.h"
#include "summary.h"

#define INIT	0
#define	TITLE	1
#define	BODY	2


char	section;
char	*href_ptr=NULL, *name_ptr=NULL, *content_ptr=NULL;
int	href_attr=1, name_attr=2, content_attr=4;

typedef struct
{
    char	*data;
    int		pos, maxsize;
    char	overflow;
} buffer;

// pruker ikke globala variabler foo dette. Sender i steden med en sturft av typen "struct bufferformat"
//buffer		title, body, metakeyw, metadesc;
//buffer		*current_buffer;

struct bufferformat {
	buffer 		title;
	buffer 		body;
	buffer	 	metakeyw;
	buffer 		metadesc;
	buffer          *current_buffer;	
};

char* translate(char *s);
void print_with_escapes(char *c,struct bufferformat *buffers);




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

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 219 of yacc.c.  */
#line 179 "y.tab.c"

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T) && (defined (__STDC__) || defined (__cplusplus))
# include <stddef.h> /* INFRINGES ON USER NAME SPACE */
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

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

#if ! defined (yyoverflow) || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if defined (__STDC__) || defined (__cplusplus)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     define YYINCLUDED_STDLIB_H
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2005 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM ((YYSIZE_T) -1)
#  endif
#  ifdef __cplusplus
extern "C" {
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if (! defined (malloc) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if (! defined (free) && ! defined (YYINCLUDED_STDLIB_H) \
	&& (defined (__STDC__) || defined (__cplusplus)))
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifdef __cplusplus
}
#  endif
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short int yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short int) + sizeof (YYSTYPE))			\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
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
      while (0)
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
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short int yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   14

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  12
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  10
/* YYNRULES -- Number of rules. */
#define YYNRULES  18
/* YYNRULES -- Number of states. */
#define YYNSTATES  24

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   266

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    13,    15,    17,
      22,    26,    31,    32,    35,    39,    43,    45,    47
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      13,     0,    -1,    -1,    13,    14,    -1,    15,    -1,    21,
      -1,    16,    -1,    17,    -1,    18,    -1,     4,     9,    19,
       5,    -1,     7,     9,     8,    -1,     4,     9,    19,     6,
      -1,    -1,    19,    20,    -1,     9,    10,    11,    -1,     9,
      10,     9,    -1,     9,    -1,    11,    -1,     3,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned char yyrline[] =
{
       0,    65,    65,    66,    68,    69,    71,    72,    73,    75,
     103,   112,   134,   135,   138,   164,   184,   186,   189
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "WORD", "TAG_START", "TAG_STOPP",
  "TAG_ENDTAG_STOPP", "ENDTAG_START", "ENDTAG_STOPP", "ATTR", "EQUALS",
  "TEXTFIELD", "$accept", "doc", "block", "tag", "starttag", "endtag",
  "startendtag", "attrlist", "attr", "text", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short int yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    12,    13,    13,    14,    14,    15,    15,    15,    16,
      17,    18,    19,    19,    20,    20,    20,    20,    21
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     0,     2,     1,     1,     1,     1,     1,     4,
       3,     4,     0,     2,     3,     3,     1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       2,     0,     1,    18,     0,     0,     3,     4,     6,     7,
       8,     5,    12,     0,     0,    10,     9,    11,    16,    17,
      13,     0,    15,    14
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     1,     6,     7,     8,     9,    10,    14,    20,    11
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -9
static const yysigned_char yypact[] =
{
      -9,     0,    -9,    -9,    -8,    -7,    -9,    -9,    -9,    -9,
      -9,    -9,    -9,    -3,     3,    -9,    -9,    -9,    -4,    -9,
      -9,     2,    -9,    -9
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
      -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9,    -9
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
       2,    12,    13,     3,     4,    15,    21,     5,    16,    17,
       0,    22,    18,    23,    19
};

static const yysigned_char yycheck[] =
{
       0,     9,     9,     3,     4,     8,    10,     7,     5,     6,
      -1,     9,     9,    11,    11
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    13,     0,     3,     4,     7,    14,    15,    16,    17,
      18,    21,     9,     9,    19,     8,     5,     6,     9,    11,
      20,    10,     9,    11
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
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (0)


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)								\
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
    while (0)
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
} while (0)

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr,					\
                  Type, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short int *bottom, short int *top)
#else
static void
yy_stack_print (bottom, top)
    short int *bottom;
    short int *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname[yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

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
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
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
      size_t yyn = 0;
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

#endif /* YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);


# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
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
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

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
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (void)
#else
int
yyparse ()
    ;
#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short int yyssa[YYINITDEPTH];
  short int *yyss = yyssa;
  short int *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

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
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short int *yyss1 = yyss;


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
	short int *yyss1 = yyss;
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

/* Do appropriate processing given the current state.  */
/* Read a look-ahead token if we need one and don't already have one.  */
/* yyresume: */

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

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
        case 9:
#line 76 "summary.yacc"
    {
		if (section==INIT && !strcasecmp("title",(char*)(yyvsp[-2]))) section = TITLE;
		else if ((section==INIT||section==TITLE) && !strcasecmp("body",(char*)(yyvsp[-2])))
		    {
			section = BODY;
			//current_buffer = &buffers.body;
			((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->body;
		    }
		else if (!strcasecmp("meta",(char*)(yyvsp[-2])) && (((yyvsp[-1]) & name_attr)>0) && (((yyvsp[-1]) & content_attr)>0))
		    {
			if (!strcasecmp("keywords",name_ptr))
			    {
				buffer	*old_buf = ((struct bufferformat *) buffers)->current_buffer;
				((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->metakeyw;
				print_with_escapes( translate(content_ptr),(struct bufferformat *) buffers );
				((struct bufferformat *) buffers)->current_buffer = old_buf;
			    }
			else if (!strcasecmp("description",name_ptr))
			    {
				buffer	*old_buf = ((struct bufferformat *) buffers)->current_buffer;
				((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->metadesc;
				print_with_escapes( translate(content_ptr) , (struct bufferformat *) buffers);
				((struct bufferformat *) buffers)->current_buffer = old_buf;
			    }
		    }
	    }
    break;

  case 10:
#line 104 "summary.yacc"
    {
		if (section==TITLE && !strcasecmp("title",(char*)(yyvsp[-1])))
		    {
			section = BODY;
			((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->body;
		    }
	    }
    break;

  case 11:
#line 113 "summary.yacc"
    {
		if (!strcasecmp("meta",(char*)(yyvsp[-2])) && (((yyvsp[-1]) & name_attr)>0) && (((yyvsp[-1]) & content_attr)>0))
		    {
			if (!strcasecmp("keywords",name_ptr))
			    {
				buffer	*old_buf = ((struct bufferformat *) buffers)->current_buffer;
				((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->metakeyw;
				print_with_escapes( translate(content_ptr), (struct bufferformat *) buffers );
				((struct bufferformat *) buffers)->current_buffer = old_buf;
			    }
			else if (!strcasecmp("description",name_ptr))
			    {
				buffer	*old_buf = ((struct bufferformat *) buffers)->current_buffer;
				((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->metadesc;
				print_with_escapes( translate(content_ptr),(struct bufferformat *) buffers);
				((struct bufferformat *) buffers)->current_buffer = old_buf;
			    }
		    }
	    }
    break;

  case 12:
#line 134 "summary.yacc"
    { (yyval) = 0; }
    break;

  case 13:
#line 136 "summary.yacc"
    { (yyval) = (yyvsp[-1]) | (yyvsp[0]); }
    break;

  case 14:
#line 139 "summary.yacc"
    {
		if (!strcasecmp("href",(char*)(yyvsp[-2])))
		    {
			href_ptr = (char*)(yyvsp[0]);
			(yyval) = href_attr;
			href_ptr++;
			href_ptr[strlen(href_ptr)-1] = '\0';
		    }
		else if (!strcasecmp("name",(char*)(yyvsp[-2])))
		    {
			name_ptr = (char*)(yyvsp[0]);
			(yyval) = name_attr;
			name_ptr++;
			name_ptr[strlen(name_ptr)-1] = '\0';
		    }
		else if (!strcasecmp("content",(char*)(yyvsp[-2])))
		    {
			content_ptr = (char*)(yyvsp[0]);
			(yyval) = content_attr;
			content_ptr++;
			content_ptr[strlen(content_ptr)-1] = '\0';
		    }
		else
		    (yyval) = 0;
	    }
    break;

  case 15:
#line 165 "summary.yacc"
    {
		if (!strcasecmp("href",(char*)(yyvsp[-2])))
		    {
			href_ptr = (char*)(yyvsp[0]);
			(yyval) = href_attr;
		    }
		else if (!strcasecmp("name",(char*)(yyvsp[-2])))
		    {
			name_ptr = (char*)(yyvsp[0]);
			(yyval) = name_attr;
		    }
		else if (!strcasecmp("content",(char*)(yyvsp[-2])))
		    {
			content_ptr = (char*)(yyvsp[0]);
			(yyval) = content_attr;
		    }
		else
		    (yyval) = 0;
	    }
    break;

  case 16:
#line 185 "summary.yacc"
    { (yyval) = 0; }
    break;

  case 17:
#line 187 "summary.yacc"
    { (yyval) = 0; }
    break;

  case 18:
#line 190 "summary.yacc"
    {
		if (section==INIT)
		    {
			section = BODY;
			((struct bufferformat *) buffers)->current_buffer = &((struct bufferformat *) buffers)->body;
		    }

		// Translate escapes first, to ensure no illegal escapes,
		// then retranslate back to escapes:
		print_with_escapes( translate((char*)(yyvsp[0])),(struct bufferformat *) buffers );
	    }
    break;


      default: break;
    }

/* Line 1126 of yacc.c.  */
#line 1323 "y.tab.c"

  yyvsp -= yylen;
  yyssp -= yylen;


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
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  int yytype = YYTRANSLATE (yychar);
	  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
	  YYSIZE_T yysize = yysize0;
	  YYSIZE_T yysize1;
	  int yysize_overflow = 0;
	  char *yymsg = 0;
#	  define YYERROR_VERBOSE_ARGS_MAXIMUM 5
	  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
	  int yyx;

#if 0
	  /* This is so xgettext sees the translatable formats that are
	     constructed on the fly.  */
	  YY_("syntax error, unexpected %s");
	  YY_("syntax error, unexpected %s, expecting %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s");
	  YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
#endif
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
	  int yychecklim = YYLAST - yyn;
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
		yysize_overflow |= yysize1 < yysize;
		yysize = yysize1;
		yyfmt = yystpcpy (yyfmt, yyprefix);
		yyprefix = yyor;
	      }

	  yyf = YY_(yyformat);
	  yysize1 = yysize + yystrlen (yyf);
	  yysize_overflow |= yysize1 < yysize;
	  yysize = yysize1;

	  if (!yysize_overflow && yysize <= YYSTACK_ALLOC_MAXIMUM)
	    yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg)
	    {
	      /* Avoid sprintf, as that infringes on the user's name space.
		 Don't have undefined behavior even if the translation
		 produced a string with the wrong number of "%s"s.  */
	      char *yyp = yymsg;
	      int yyi = 0;
	      while ((*yyp = *yyf))
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
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    {
	      yyerror (YY_("syntax error"));
	      goto yyexhaustedlab;
	    }
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (YY_("syntax error"));
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
	  yydestruct ("Error: discarding", yytoken, &yylval);
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
  if (0)
     goto yyerrorlab;

yyvsp -= yylen;
  yyssp -= yylen;
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


      yydestruct ("Error: popping", yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


  /* Shift the error token. */
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
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK;
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 202 "summary.yacc"


extern FILE *yyin;
extern int linenr;



struct trans_tab
{
    char	*escape, translation;
};

struct trans_tab tt[130] = {
    {"#178",'²'},{"#179",'³'},{"#185",'¹'},{"#192",'À'},{"#193",'Á'},{"#194",'Â'},{"#195",'Ã'},{"#196",'Ä'},
    {"#197",'Å'},{"#198",'Æ'},{"#199",'Ç'},{"#200",'È'},{"#201",'É'},{"#202",'Ê'},{"#203",'Ë'},{"#204",'Ì'},
    {"#205",'Í'},{"#206",'Î'},{"#207",'Ï'},{"#208",'Ð'},{"#209",'Ñ'},{"#210",'Ò'},{"#211",'Ó'},{"#212",'Ô'},
    {"#213",'Õ'},{"#214",'Ö'},{"#216",'Ø'},{"#217",'Ù'},{"#218",'Ú'},{"#219",'Û'},{"#220",'Ü'},{"#221",'Ý'},
    {"#222",'Þ'},{"#223",'ß'},{"#224",'à'},{"#225",'á'},{"#226",'â'},{"#227",'ã'},{"#228",'ä'},{"#229",'å'},
    {"#230",'æ'},{"#231",'ç'},{"#232",'è'},{"#233",'é'},{"#234",'ê'},{"#235",'ë'},{"#236",'ì'},{"#237",'í'},
    {"#238",'î'},{"#239",'ï'},{"#240",'ð'},{"#241",'ñ'},{"#242",'ò'},{"#243",'ó'},{"#244",'ô'},{"#245",'õ'},
    {"#246",'ö'},{"#248",'ø'},{"#249",'ù'},{"#250",'ú'},{"#251",'û'},{"#252",'ü'},{"#253",'ý'},{"#254",'þ'},
    {"#255",'ÿ'},{"AElig",'Æ'},{"Aacute",'Á'},{"Acirc",'Â'},{"Agrave",'À'},{"Aring",'Å'},{"Atilde",'Ã'},{"Auml",'Ä'},
    {"Ccedil",'Ç'},{"ETH",'Ð'},{"Eacute",'É'},{"Ecirc",'Ê'},{"Egrave",'È'},{"Euml",'Ë'},{"Iacute",'Í'},{"Icirc",'Î'},
    {"Igrave",'Ì'},{"Iuml",'Ï'},{"Ntilde",'Ñ'},{"Oacute",'Ó'},{"Ocirc",'Ô'},{"Ograve",'Ò'},{"Oslash",'Ø'},{"Otilde",'Õ'},
    {"Ouml",'Ö'},{"THORN",'Þ'},{"Uacute",'Ú'},{"Ucirc",'Û'},{"Ugrave",'Ù'},{"Uuml",'Ü'},{"Yacute",'Ý'},{"aacute",'á'},
    {"acirc",'â'},{"aelig",'æ'},{"agrave",'à'},{"aring",'å'},{"atilde",'ã'},{"auml",'ä'},{"ccedil",'ç'},{"eacute",'é'},
    {"ecirc",'ê'},{"egrave",'è'},{"eth",'ð'},{"euml",'ë'},{"iacute",'í'},{"icirc",'î'},{"igrave",'ì'},{"iuml",'ï'},
    {"ntilde",'ñ'},{"oacute",'ó'},{"ocirc",'ô'},{"ograve",'ò'},{"oslash",'ø'},{"otilde",'õ'},{"ouml",'ö'},{"sup1",'¹'},
    {"sup2",'²'},{"sup3",'³'},{"szlig",'ß'},{"thorn",'þ'},{"uacute",'ú'},{"ucirc",'û'},{"ugrave",'ù'},{"uuml",'ü'},
    {"yacute",'ý'},{"yuml",'ÿ'}};


int compare(const void *a, const void *b)
{
    return strncmp( (char*)a, ((struct trans_tab*)b)->escape, strlen(((struct trans_tab*)b)->escape) );
}

// Translate escapes in string:
char* translate(char *s)
{
    char	*d = (char*)malloc(strlen(s)+1);
    int		i, j, k;
    char	replace;

    for (i=0, j=0; s[j]!='\0';)
	switch (s[j])
	    {
		case '&':
		    replace = 0;

		    if (s[j+1]!='\0')
			{
			    struct trans_tab	*code = (struct trans_tab*)bsearch(&(s[j+1]),tt,130,sizeof(struct trans_tab),compare);

			    if (code!=NULL)
				{
				    replace = 1;
				    d[i++] = code->translation;
				    j+= strlen(code->escape)+1;
				    if (s[j]==';') j++;
				}
			}

		    if (!replace)
			{
			    d[i++] = '&';
			    j++;
			}

		    break;
		default:
		    d[i++] = s[j++];
	    }

end:
    d[i] = '\0';
    return d;
}


struct html_esc
{
    char	c, *esc;
};

struct html_esc he[65] = {
    {'²',"sup2"},{'³',"sup3"},{'¹',"sup1"},{'À',"Agrave"},{'Á',"Aacute"},{'Â',"Acirc"},{'Ã',"Atilde"},{'Ä',"Auml"},
    {'Å',"Aring"},{'Æ',"AElig"},{'Ç',"Ccedil"},{'È',"Egrave"},{'É',"Eacute"},{'Ê',"Ecirc"},{'Ë',"Euml"},{'Ì',"Igrave"},
    {'Í',"Iacute"},{'Î',"Icirc"},{'Ï',"Iuml"},{'Ð',"ETH"},{'Ñ',"Ntilde"},{'Ò',"Ograve"},{'Ó',"Oacute"},{'Ô',"Ocirc"},
    {'Õ',"Otilde"},{'Ö',"Ouml"},{'Ø',"Oslash"},{'Ù',"Ugrave"},{'Ú',"Uacute"},{'Û',"Ucirc"},{'Ü',"Uuml"},{'Ý',"Yacute"},
    {'Þ',"THORN"},{'ß',"szlig"},{'à',"agrave"},{'á',"aacute"},{'â',"acirc"},{'ã',"atilde"},{'ä',"auml"},{'å',"aring"},
    {'æ',"aelig"},{'ç',"ccedil"},{'è',"egrave"},{'é',"eacute"},{'ê',"ecirc"},{'ë',"euml"},{'ì',"igrave"},{'í',"iacute"},
    {'î',"icirc"},{'ï',"iuml"},{'ð',"eth"},{'ñ',"ntilde"},{'ò',"ograve"},{'ó',"oacute"},{'ô',"ocirc"},{'õ',"otilde"},
    {'ö',"ouml"},{'ø',"oslash"},{'ù',"ugrave"},{'ú',"uacute"},{'û',"ucirc"},{'ü',"uuml"},{'ý',"yacute"},{'þ',"thorn"},
    {'ÿ',"yuml"}};


int esc_compare(const void *a, const void *b)
{
    if (*((char*)a) < ((struct html_esc*)b)->c) return -1;
    if (*((char*)a) > ((struct html_esc*)b)->c) return +1;
    return 0;
}

void print2buffer(struct bufferformat *buffers,const char *fmt, ...)
{
    if (((struct bufferformat *) buffers)->current_buffer->overflow) return;

    va_list	ap;

    va_start(ap, fmt);
    int	len_printed = vsnprintf(&(((struct bufferformat *) buffers)->current_buffer->data[((struct bufferformat *) buffers)->current_buffer->pos]), ((struct bufferformat *) buffers)->current_buffer->maxsize - ((struct bufferformat *) buffers)->current_buffer->pos - 1, fmt, ap);

    ((struct bufferformat *) buffers)->current_buffer->pos+= len_printed;

//    if (section==TITLE) title_size+= len_printed;
//    else body_size+= len_printed;

    if (((struct bufferformat *) buffers)->current_buffer->pos >= ((struct bufferformat *) buffers)->current_buffer->maxsize - 1) ((struct bufferformat *) buffers)->current_buffer->overflow = 1;
}


void print_with_escapes(char *c,struct bufferformat *buffers)
{
    int		i;

    if (((struct bufferformat *) buffers)->current_buffer->pos > 0) print2buffer((struct bufferformat *) buffers," ");

    for (i=0; c[i]!='\0'; i++)
	if ((unsigned char)c[i]<128)
	    print2buffer((struct bufferformat *) buffers,"%c", c[i]);
	else
	    {
		struct html_esc	*p = (struct html_esc*)
		bsearch( (const void*)(((char*)&(c[i]))), he, 65, sizeof(struct html_esc), esc_compare);

		if (p==NULL)
		    print2buffer((struct bufferformat *) buffers,"%c", c[i]);
		else
		    print2buffer((struct bufferformat *) buffers,"&%s;", p->esc);
	    }

    free( c );
}

buffer buffer_init( int _maxsize )
{
    buffer	b;

    b.overflow = 0;
    b.pos = 0;
    b.maxsize = _maxsize;
    b.data = (char*)malloc(b.maxsize);

    return b;
}

char* buffer_exit( buffer b )
{
    char	*output;

    output = (char*)malloc(b.pos+1);
    memcpy( output, &(b.data[0]), b.pos );
    output[b.pos] = '\0';
    free( b.data );

    return output;
}

void generate_summary( char text[], int text_size, char **output_title, char **output_body, char **output_metakeywords, char **output_metadescription )
{
    // Set global variables for lexer:
    custom_input = text;
    custom_pos = 0;
    custom_size = text_size;

    // Set variables for yacc-er:
    section = INIT;

    // We accept output summaries of a size up to double the original textsize.
    // (Although on almost all occations the size will shrink).
//    maxsize = text_size*2;

    // Allocate output_buffer:
//    output_buffer = (char*)malloc(maxsize);
//    title_size = body_size = pos = 0;
//    overflow = 0;	// To prevent corrupt inputfiles to overflow output_buffer.

    // Fields 'title', 'meta keywords' and 'meta description', will only keep first 10240 bytes,
    // field body will only keep up to double original textsize (should be enough for all ordinary documents).

    struct bufferformat buffers;

    buffers.title = buffer_init( 10240 );
    buffers.body = buffer_init( text_size*2 );
    buffers.metakeyw = buffer_init( 10240 );
    buffers.metadesc = buffer_init( 10240 );

    buffers.current_buffer = &buffers.title;

    // Run parser:
    do
	{
	    yyparse((void*)&buffers);
	}
    while (custom_pos<custom_size);

    (*output_title) = buffer_exit( buffers.title );
    (*output_body) = buffer_exit( buffers.body );
    (*output_metakeywords) = buffer_exit( buffers.metakeyw );
    (*output_metadescription) = buffer_exit( buffers.metadesc );
}


yyerror( char *s )
{
    //fprintf( stderr, "parse_error %s on line %i\n", s, linenr );
}

