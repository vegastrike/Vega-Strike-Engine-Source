
/*  A Bison parser, made from c_alike.yacc
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	L_ID	257
#define	L_FLOATCONST	258
#define	L_INTCONST	259
#define	L_BOOLCONST_TRUE	260
#define	L_BOOLCONST_FALSE	261
#define	L_MODULE	262
#define	L_SCRIPT	263
#define	L_IMPORT	264
#define	L_RETURN	265
#define	L_GLOBALS	266
#define	L_IF	267
#define	L_THEN	268
#define	L_ELSE	269
#define	L_WHILE	270
#define	L_EQUAL	271
#define	L_NOT_EQUAL	272
#define	L_GREATER_OR_EQUAL	273
#define	L_LESSER_OR_EQUAL	274
#define	L_BOOL_AND	275
#define	L_BOOL_OR	276
#define	L_INT	277
#define	L_FLOAT	278
#define	L_BOOL	279
#define	L_OBJECT	280
#define	L_VOID	281
#define	L_BEGINSCRIPT	282
#define	L_ENDSCRIPT	283
#define	L_BEGINBLOCK	284
#define	L_ENDBLOCK	285

#line 1 "c_alike.yacc"

#include <stdio.h>
#include <malloc.h>

#include <vector>
#include <string>

#define YYERROR_VERBOSE

extern int yyerror(char *);
extern int yywrap();
extern int yylex();

#define YYDEBUG 0

  typedef char* str;
#define YYSTYPE string

string module_string;

#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		124
#define	YYFLAG		-32768
#define	YYNTBASE	47

#define YYTRANSLATE(x) ((unsigned)(x) <= 285 ? yytranslate[x] : 77)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    46,     2,     2,     2,     2,     2,     2,    40,
    41,    34,    33,    43,    32,    42,    35,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,    38,    44,
    39,    45,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    36,     2,    37,     2,     2,     2,     2,     2,
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
     2,     2,     2,     2,     2,     1,     3,     4,     5,     6,
     7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
    17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
    27,    28,    29,    30,    31
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     6,     7,    11,    13,    15,    17,    19,    24,    25,
    29,    31,    34,    36,    38,    40,    42,    44,    46,    48,
    50,    52,    57,    60,    65,    71,    72,    76,    78,    79,
    83,    85,    87,    89,    91,    93,    95,    97,    99,   102,
   106,   113,   118,   119,   123,   127,   135,   139,   141,   143,
   145,   147,   149,   151,   153,   155,   159,   161,   165,   169,
   173,   177,   181,   185,   189,   193,   197,   201,   205,   209,
   212
};

static const short yyrhs[] = {     8,
     3,    36,    48,    37,     0,     0,    48,    49,    38,     0,
    60,     0,    59,     0,    53,     0,    50,     0,    12,    36,
    51,    37,     0,     0,    51,    52,    38,     0,    59,     0,
    10,     3,     0,    55,     0,    56,     0,    57,     0,    58,
     0,    23,     0,    24,     0,    25,     0,    26,     0,    27,
     0,    54,     3,    39,    73,     0,    54,     3,     0,    61,
    36,    64,    37,     0,    54,     3,    40,    62,    41,     0,
     0,    62,    63,    38,     0,    59,     0,     0,    64,    65,
    38,     0,    71,     0,    72,     0,    59,     0,    70,     0,
    68,     0,    67,     0,    66,     0,    11,     0,    11,    76,
     0,    16,    76,    72,     0,     3,    42,     3,    40,    69,
    41,     0,     3,    40,    69,    41,     0,     0,    69,    76,
    43,     0,     3,    39,    76,     0,    13,    40,    76,    41,
    72,    15,    72,     0,    36,    64,    37,     0,    75,     0,
    74,     0,     6,     0,     7,     0,     4,     0,     5,     0,
    73,     0,     3,     0,     3,    42,     3,     0,    68,     0,
    76,    17,    76,     0,    76,    18,    76,     0,    76,    20,
    76,     0,    76,    19,    76,     0,    76,    44,    76,     0,
    76,    45,    76,     0,    76,    34,    76,     0,    76,    35,
    76,     0,    76,    32,    76,     0,    76,    33,    76,     0,
    76,    21,    76,     0,    76,    22,    76,     0,    46,    76,
     0,    40,    76,    41,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    39,    44,    45,    48,    48,    48,    48,    49,    52,    53,
    56,    57,    60,    60,    60,    60,    62,    63,    64,    65,
    66,    67,    70,    73,    76,    79,    80,    83,    86,    88,
    92,    93,    94,    95,    96,    97,    98,    99,   102,   106,
   109,   117,   120,   121,   124,   127,   130,   133,   133,   134,
   135,   136,   139,   142,   143,   146,   149,   150,   152,   154,
   156,   158,   160,   163,   165,   167,   169,   171,   173,   175,
   177
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","L_ID","L_FLOATCONST",
"L_INTCONST","L_BOOLCONST_TRUE","L_BOOLCONST_FALSE","L_MODULE","L_SCRIPT","L_IMPORT",
"L_RETURN","L_GLOBALS","L_IF","L_THEN","L_ELSE","L_WHILE","L_EQUAL","L_NOT_EQUAL",
"L_GREATER_OR_EQUAL","L_LESSER_OR_EQUAL","L_BOOL_AND","L_BOOL_OR","L_INT","L_FLOAT",
"L_BOOL","L_OBJECT","L_VOID","L_BEGINSCRIPT","L_ENDSCRIPT","L_BEGINBLOCK","L_ENDBLOCK",
"'-'","'+'","'*'","'/'","'{'","'}'","';'","'='","'('","')'","'.'","','","'<'",
"'>'","'!'","module","module_body","module_statement","globals","globals_body",
"global_statement","import","vartype","inttype","floattype","booltype","objecttype",
"defvar","script","script_header","arguments","argument","script_body","script_statement",
"return_statement","while_statement","call_void","call_arglist","setvar","if_statement",
"block_statement","constant","boolconst","number","expr", NULL
};
#endif

static const short yyr1[] = {     0,
    47,    48,    48,    49,    49,    49,    49,    50,    51,    51,
    52,    53,    54,    54,    54,    54,    55,    56,    57,    58,
    -1,    59,    59,    60,    61,    62,    62,    63,    64,    64,
    65,    65,    65,    65,    65,    65,    65,    66,    66,    67,
    68,    68,    69,    69,    70,    71,    72,    73,    73,    74,
    74,    75,    75,    76,    76,    76,    76,    76,    76,    76,
    76,    76,    76,    76,    76,    76,    76,    76,    76,    76,
    76
};

static const short yyr2[] = {     0,
     5,     0,     3,     1,     1,     1,     1,     4,     0,     3,
     1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     4,     2,     4,     5,     0,     3,     1,     0,     3,
     1,     1,     1,     1,     1,     1,     1,     1,     2,     3,
     6,     4,     0,     3,     3,     7,     3,     1,     1,     1,
     1,     1,     1,     1,     1,     3,     1,     3,     3,     3,
     3,     3,     3,     3,     3,     3,     3,     3,     3,     2,
     3
};

static const short yydefact[] = {     0,
     0,     0,     2,     0,     0,     0,    17,    18,    19,    20,
     1,     0,     7,     6,     0,    13,    14,    15,    16,     5,
     4,     0,    12,     9,     3,    23,    29,     0,     0,    26,
     0,     8,     0,     0,    11,    52,    53,    50,    51,    22,
    49,    48,     0,     0,    38,     0,     0,    29,    24,    33,
     0,    37,    36,    35,    34,    31,    32,    10,    23,    25,
    28,     0,     0,    43,     0,    55,     0,     0,    57,    54,
    39,     0,     0,     0,    30,    27,    45,     0,     0,     0,
     0,    70,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,    40,    47,    42,     0,    43,
    56,    71,    58,    59,    61,    60,    68,    69,    66,    67,
    64,    65,    62,    63,     0,    44,     0,     0,    41,     0,
    46,     0,     0,     0
};

static const short yydefgoto[] = {   122,
     4,    12,    13,    28,    33,    14,    34,    16,    17,    18,
    19,    50,    21,    22,    43,    62,    31,    51,    52,    53,
    69,    78,    55,    56,    57,    70,    41,    42,    99
};

static const short yypact[] = {    -7,
     4,   -17,-32768,   213,    17,    18,-32768,-32768,-32768,-32768,
-32768,    15,-32768,-32768,    59,-32768,-32768,-32768,-32768,-32768,
-32768,    31,-32768,-32768,-32768,   -14,-32768,   -13,    10,-32768,
    61,-32768,    35,    65,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,   -20,    39,    30,    29,    30,-32768,-32768,-32768,
    42,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    43,-32768,
-32768,    45,    30,-32768,    85,    21,    30,    30,-32768,-32768,
   183,    30,    99,    89,-32768,-32768,   183,    25,    49,    88,
   119,   183,    30,    30,    30,    30,    30,    30,    30,    30,
    30,    30,    30,    30,   148,-32768,-32768,-32768,   177,-32768,
    49,-32768,   183,   183,   183,   183,   183,   183,   212,   212,
   128,   128,   183,   183,    60,-32768,    53,    80,-32768,    60,
-32768,   101,   103,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,-32768,   100,-32768,-32768,-32768,
-32768,    47,-32768,-32768,-32768,-32768,    58,-32768,-32768,-32768,
   -22,     7,-32768,-32768,   -65,    79,-32768,-32768,   -45
};


#define	YYLAST		257


static const short yytable[] = {    71,
     1,    73,     7,     8,     9,    10,     2,    96,    54,     7,
     8,     9,    10,    36,    37,    38,    39,    77,     3,    23,
    60,    81,    82,    32,    29,    30,    95,    66,    36,    37,
    38,    39,    66,    36,    37,    38,    39,   103,   104,   105,
   106,   107,   108,   109,   110,   111,   112,   113,   114,   118,
    20,    54,    25,    24,   121,    66,    36,    37,    38,    39,
    64,    26,    80,    44,    67,    98,    27,    59,    72,    67,
    68,    45,    58,    46,    35,    68,    47,    63,    64,    75,
    65,    29,    76,     7,     8,     9,    10,    79,   100,    61,
   101,    44,    67,   119,   120,    48,    48,    49,    68,    45,
   123,    46,   124,    15,    47,    74,   117,    40,     0,     0,
     0,     7,     8,     9,    10,    83,    84,    85,    86,    87,
    88,     0,     0,     0,    48,    97,     0,     0,     0,     0,
    89,    90,    91,    92,    48,    83,    84,    85,    86,    87,
    88,     0,    93,    94,    83,    84,    85,    86,    87,    88,
    89,    90,    91,    92,     0,     0,     0,     0,     0,   102,
     0,     0,    93,    94,    83,    84,    85,    86,    87,    88,
     0,    93,    94,     0,     0,     0,     0,     0,     0,    89,
    90,    91,    92,     0,     0,     0,     0,     0,   115,     0,
     0,    93,    94,    83,    84,    85,    86,    87,    88,    83,
    84,    85,    86,    87,    88,     0,     0,     0,    89,    90,
    91,    92,     0,     0,    89,    90,    91,    92,     0,   116,
    93,    94,     5,     0,     6,     0,    93,    94,    83,    84,
    85,    86,    87,    88,     0,     7,     8,     9,    10,     0,
     0,     0,     0,     0,     0,    91,    92,     0,     0,    11,
     0,     0,     0,     0,     0,    93,    94
};

static const short yycheck[] = {    45,
     8,    47,    23,    24,    25,    26,     3,    73,    31,    23,
    24,    25,    26,     4,     5,     6,     7,    63,    36,     3,
    41,    67,    68,    37,    39,    40,    72,     3,     4,     5,
     6,     7,     3,     4,     5,     6,     7,    83,    84,    85,
    86,    87,    88,    89,    90,    91,    92,    93,    94,   115,
     4,    74,    38,    36,   120,     3,     4,     5,     6,     7,
    40,     3,    42,     3,    40,    41,    36,     3,    40,    40,
    46,    11,    38,    13,    28,    46,    16,    39,    40,    38,
    42,    39,    38,    23,    24,    25,    26,     3,    40,    43,
     3,     3,    40,    41,    15,    36,    36,    37,    46,    11,
     0,    13,     0,     4,    16,    48,   100,    29,    -1,    -1,
    -1,    23,    24,    25,    26,    17,    18,    19,    20,    21,
    22,    -1,    -1,    -1,    36,    37,    -1,    -1,    -1,    -1,
    32,    33,    34,    35,    36,    17,    18,    19,    20,    21,
    22,    -1,    44,    45,    17,    18,    19,    20,    21,    22,
    32,    33,    34,    35,    -1,    -1,    -1,    -1,    -1,    41,
    -1,    -1,    44,    45,    17,    18,    19,    20,    21,    22,
    -1,    44,    45,    -1,    -1,    -1,    -1,    -1,    -1,    32,
    33,    34,    35,    -1,    -1,    -1,    -1,    -1,    41,    -1,
    -1,    44,    45,    17,    18,    19,    20,    21,    22,    17,
    18,    19,    20,    21,    22,    -1,    -1,    -1,    32,    33,
    34,    35,    -1,    -1,    32,    33,    34,    35,    -1,    43,
    44,    45,    10,    -1,    12,    -1,    44,    45,    17,    18,
    19,    20,    21,    22,    -1,    23,    24,    25,    26,    -1,
    -1,    -1,    -1,    -1,    -1,    34,    35,    -1,    -1,    37,
    -1,    -1,    -1,    -1,    -1,    44,    45
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/lib/bison.simple"
/* This file comes from bison-1.28.  */

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

#ifndef YYSTACK_USE_ALLOCA
#ifdef alloca
#define YYSTACK_USE_ALLOCA
#else /* alloca not defined */
#ifdef __GNUC__
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi) || (defined (__sun) && defined (__i386))
#define YYSTACK_USE_ALLOCA
#include <alloca.h>
#else /* not sparc */
/* We think this test detects Watcom and Microsoft C.  */
/* This used to test MSDOS, but that is a bad idea
   since that symbol is in the user namespace.  */
#if (defined (_MSDOS) || defined (_MSDOS_)) && !defined (__TURBOC__)
#if 0 /* No need for malloc.h, which pollutes the namespace;
	 instead, just don't use alloca.  */
#include <malloc.h>
#endif
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
/* I don't know what this was needed for, but it pollutes the namespace.
   So I turned it off.   rms, 2 May 1997.  */
/* #include <malloc.h>  */
 #pragma alloca
#define YYSTACK_USE_ALLOCA
#else /* not MSDOS, or __TURBOC__, or _AIX */
#if 0
#ifdef __hpux /* haible@ilog.fr says this works for HPUX 9.05 and up,
		 and on HPUX 10.  Eventually we can turn this on.  */
#define YYSTACK_USE_ALLOCA
#define alloca __builtin_alloca
#endif /* __hpux */
#endif
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc */
#endif /* not GNU C */
#endif /* alloca not defined */
#endif /* YYSTACK_USE_ALLOCA not defined */

#ifdef YYSTACK_USE_ALLOCA
#define YYSTACK_ALLOC alloca
#else
#define YYSTACK_ALLOC malloc
#endif

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Define __yy_memcpy.  Note that the size argument
   should be passed with type unsigned int, because that is what the non-GCC
   definitions require.  With GCC, __builtin_memcpy takes an arg
   of type size_t, but it can handle unsigned int.  */

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(TO,FROM,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (to, from, count)
     char *to;
     char *from;
     unsigned int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *to, char *from, unsigned int count)
{
  register char *t = to;
  register char *f = from;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 217 "/usr/lib/bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#ifdef __cplusplus
#define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#else /* not __cplusplus */
#define YYPARSE_PARAM_ARG YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#endif /* not __cplusplus */
#else /* not YYPARSE_PARAM */
#define YYPARSE_PARAM_ARG
#define YYPARSE_PARAM_DECL
#endif /* not YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
#ifdef YYPARSE_PARAM
int yyparse (void *);
#else
int yyparse (void);
#endif
#endif

int
yyparse(YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;
  int yyfree_stacks = 0;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  if (yyfree_stacks)
	    {
	      free (yyss);
	      free (yyvs);
#ifdef YYLSP_NEEDED
	      free (yyls);
#endif
	    }
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
#ifndef YYSTACK_USE_ALLOCA
      yyfree_stacks = 1;
#endif
      yyss = (short *) YYSTACK_ALLOC (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss, (char *)yyss1,
		   size * (unsigned int) sizeof (*yyssp));
      yyvs = (YYSTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs, (char *)yyvs1,
		   size * (unsigned int) sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) YYSTACK_ALLOC (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls, (char *)yyls1,
		   size * (unsigned int) sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 1:
#line 39 "c_alike.yacc"
{
	string module="<module name="+yyvsp[-3]+" >\n"+yyvsp[-1]+"\n</module>\n";
	//	printf("%s\n",module.c_str());
	module_string=module;
;
    break;}
case 2:
#line 44 "c_alike.yacc"
{ yyval=""; ;
    break;}
case 3:
#line 45 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"\n"+yyvsp[-1];
;
    break;}
case 4:
#line 48 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 5:
#line 48 "c_alike.yacc"
{yyval=yyvsp[0];
    break;}
case 6:
#line 48 "c_alike.yacc"
{ yyval=yyvsp[0] ;
    break;}
case 7:
#line 48 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 8:
#line 49 "c_alike.yacc"
{
	yyval="<globals>\n"+yyvsp[-1]+"\n</globals>\n";
;
    break;}
case 9:
#line 52 "c_alike.yacc"
{ yyval=""; ;
    break;}
case 10:
#line 53 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"\n"+yyvsp[-1];
;
    break;}
case 11:
#line 56 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 12:
#line 57 "c_alike.yacc"
{
	yyval="<import name="+yyvsp[0]+"/>";
 ;
    break;}
case 17:
#line 62 "c_alike.yacc"
{ yyval="int" ;
    break;}
case 18:
#line 63 "c_alike.yacc"
{ yyval="float";
    break;}
case 19:
#line 64 "c_alike.yacc"
{  yyval="bool";
    break;}
case 20:
#line 65 "c_alike.yacc"
{ yyval="object";
    break;}
case 21:
#line 66 "c_alike.yacc"
{ yyval="void";;
    break;}
case 22:
#line 67 "c_alike.yacc"
{
	yyval="<defvar name="+yyvsp[-2]+" type="+yyvsp[-3]+" value="+yyvsp[0]+" />\n";
;
    break;}
case 23:
#line 70 "c_alike.yacc"
{
	yyval="<defvar name="+yyvsp[0]+" type="+yyvsp[-1]+"/>\n";
;
    break;}
case 24:
#line 73 "c_alike.yacc"
{
	yyval=yyvsp[-3]+"\n"+yyvsp[-1]+"\n</script>\n";
;
    break;}
case 25:
#line 76 "c_alike.yacc"
{
	yyval="<script name="+yyvsp[-3]+" return="+yyvsp[-4]+" >\n"+"<arguments>\n"+yyvsp[-1]+"\n</arguments>\n";
;
    break;}
case 26:
#line 79 "c_alike.yacc"
{ yyval="\n"; ;
    break;}
case 27:
#line 80 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"\n"+yyvsp[-1];
;
    break;}
case 28:
#line 83 "c_alike.yacc"
{
	yyval=yyvsp[0];
;
    break;}
case 29:
#line 87 "c_alike.yacc"
{ yyval=""; ;
    break;}
case 30:
#line 88 "c_alike.yacc"
{
//	$$="\nscript_body\n";
	yyval=yyvsp[-2]+"\n"+yyvsp[-1];
;
    break;}
case 31:
#line 92 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 32:
#line 93 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 33:
#line 94 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 34:
#line 95 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 35:
#line 96 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 36:
#line 97 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 37:
#line 98 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 38:
#line 99 "c_alike.yacc"
{
	yyval="<return/>\n";
;
    break;}
case 39:
#line 102 "c_alike.yacc"
{
	yyval="<return>\n"+yyvsp[0]+"\n</return>\n";
;
    break;}
case 40:
#line 106 "c_alike.yacc"
{
	yyval="<while>\n"+yyvsp[-1]+"\n"+yyvsp[0]+"\n</while>\n";
;
    break;}
case 41:
#line 109 "c_alike.yacc"
{
	if(yyvsp[-5][0]=='_'){
		yyval="<call module="+yyvsp[-5]+" name="+yyvsp[-3]+" >\n"+yyvsp[-1]+"\n</call>\n";
	}
	else{	
		yyval="<exec module="+yyvsp[-5]+" name="+yyvsp[-3]+" >\n"+yyvsp[-1]+"\n</exec>\n";
	}
;
    break;}
case 42:
#line 117 "c_alike.yacc"
{
	yyval="<exec name="+yyvsp[-3]+" >\n"+yyvsp[-1]+"\n</exec>\n";
;
    break;}
case 43:
#line 120 "c_alike.yacc"
{ yyval="\n"; ;
    break;}
case 44:
#line 121 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"\n"+yyvsp[-1];
;
    break;}
case 45:
#line 124 "c_alike.yacc"
{
	yyval="<setvar name="+yyvsp[-2]+" >\n"+yyvsp[0]+"\n</setvar>\n";
;
    break;}
case 46:
#line 127 "c_alike.yacc"
{
	yyval="<if>\n"+yyvsp[-4]+"\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</if>\n";
;
    break;}
case 47:
#line 130 "c_alike.yacc"
{
	yyval="<block>\n"+yyvsp[-1]+"\n</block>\n";
;
    break;}
case 48:
#line 133 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 49:
#line 133 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 50:
#line 134 "c_alike.yacc"
{ yyval="true"; ;
    break;}
case 51:
#line 135 "c_alike.yacc"
{ yyval="false"; ;
    break;}
case 52:
#line 136 "c_alike.yacc"
{
	 yyval="<const type=float value="+yyvsp[0]+" />\n";
 ;
    break;}
case 53:
#line 139 "c_alike.yacc"
{
	 yyval="<const type=int value="+yyvsp[0]+" />\n";
;
    break;}
case 54:
#line 142 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 55:
#line 143 "c_alike.yacc"
{
	yyval="<var name="+yyvsp[0]+" />\n";
;
    break;}
case 56:
#line 146 "c_alike.yacc"
{
	yyval="<var module="+yyvsp[-2]+" name="+yyvsp[-1]+" />\n";
;
    break;}
case 57:
#line 149 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 58:
#line 151 "c_alike.yacc"
{ yyval="<test test=eq >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 59:
#line 153 "c_alike.yacc"
{ yyval="<test test=ne >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 60:
#line 155 "c_alike.yacc"
{ yyval="<test test=le >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 61:
#line 157 "c_alike.yacc"
{ yyval="<test test=ge >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 62:
#line 159 "c_alike.yacc"
{ yyval="<test test=lt >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 63:
#line 161 "c_alike.yacc"
{ yyval="<test test=gt >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 64:
#line 164 "c_alike.yacc"
{ yyval="<fmath math=* >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</fmath>\n"; ;
    break;}
case 65:
#line 166 "c_alike.yacc"
{ yyval="<fmath math=/ >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</fmath>\n"; ;
    break;}
case 66:
#line 168 "c_alike.yacc"
{ yyval="<fmath math=- >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</fmath>\n"; ;
    break;}
case 67:
#line 170 "c_alike.yacc"
{ yyval="<fmath math=+ >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</fmath>\n"; ;
    break;}
case 68:
#line 172 "c_alike.yacc"
{ yyval="<and>\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</and>\n"; ;
    break;}
case 69:
#line 174 "c_alike.yacc"
{ yyval="<or>\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</or>\n"; ;
    break;}
case 70:
#line 176 "c_alike.yacc"
{ yyval="<not>\n"+yyvsp[0]+"\n</not>\n";		;
    break;}
case 71:
#line 178 "c_alike.yacc"
{ yyval=yyvsp[-1]; ;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 543 "/usr/lib/bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;

 yyacceptlab:
  /* YYACCEPT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 0;

 yyabortlab:
  /* YYABORT comes here.  */
  if (yyfree_stacks)
    {
      free (yyss);
      free (yyvs);
#ifdef YYLSP_NEEDED
      free (yyls);
#endif
    }
  return 1;
}
#line 180 "c_alike.yacc"


/* nothing */


/* extern int yydebug; */

#if 0
void main()
{
  extern FILE *yyin;
  yyin=fopen("test.c","r");
/*  yydebug=1; */


  yyparse();

}
#endif

string parseCalike(char const *filename)
{
  string empty_string;

  extern FILE *yyin;
  yyin=fopen(filename,"r");

  if(yyin==NULL){
    // file not found
    return empty_string;
  }

/*  yydebug=1; */
  yyparse();
  
  return module_string;
}

extern	int yylineno;
extern char *yytext;


int yyerror(char *s){
  printf("(yy)error: %s line %d text -%s-\n",s,yylineno,yytext);
  return 1;
}

int yywrap(){
	return 1;
}
