
/*  A Bison parser, made from c_alike.yacc
    by GNU Bison version 1.28  */

#define YYBISON 1  /* Identify Bison output.  */

#define	L_ID	257
#define	L_FLOATCONST	258
#define	L_INTCONST	259
#define	L_STRINGCONST	260
#define	L_BOOLCONST_TRUE	261
#define	L_BOOLCONST_FALSE	262
#define	L_MODULE	263
#define	L_SCRIPT	264
#define	L_IMPORT	265
#define	L_RETURN	266
#define	L_GLOBALS	267
#define	L_IF	268
#define	L_THEN	269
#define	L_ELSE	270
#define	L_WHILE	271
#define	L_EQUAL	272
#define	L_NOT_EQUAL	273
#define	L_GREATER_OR_EQUAL	274
#define	L_LESSER_OR_EQUAL	275
#define	L_BOOL_AND	276
#define	L_BOOL_OR	277
#define	L_INT	278
#define	L_FLOAT	279
#define	L_BOOL	280
#define	L_OBJECT	281
#define	L_VOID	282
#define	L_METHODCALL	283
#define	L_INITVALUE	284

#line 1 "c_alike.yacc"

/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * http://vegastrike.sourceforge.net/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
  c_alike scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
*/


#include "c_alike.h"

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



#define	YYFINAL		164
#define	YYFLAG		-32768
#define	YYNTBASE	47

#define YYTRANSLATE(x) ((unsigned)(x) <= 284 ? yytranslate[x] : 88)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    35,     2,     2,     2,     2,     2,     2,    41,
    42,    33,    32,    40,    31,    43,    34,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    44,    38,    45,
    39,    46,     2,     2,     2,     2,     2,     2,     2,     2,
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
    27,    28,    29,    30
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     6,     7,    11,    13,    15,    17,    19,    24,    25,
    29,    31,    34,    36,    38,    40,    42,    44,    46,    48,
    50,    52,    54,    57,    62,    67,    70,    75,    77,    81,
    86,    92,    94,    96,    97,    99,   101,   105,   107,   108,
   111,   113,   115,   118,   121,   124,   126,   129,   131,   133,
   136,   140,   148,   156,   161,   162,   164,   166,   170,   171,
   175,   180,   184,   192,   200,   206,   210,   212,   214,   216,
   218,   220,   222,   224,   226,   228,   230,   232,   234,   236,
   238,   240,   244,   246,   250,   254,   258,   262,   266,   270,
   274,   278,   282,   286,   290,   294,   297
};

static const short yyrhs[] = {     9,
     3,    36,    48,    37,     0,     0,    48,    49,    38,     0,
    63,     0,    61,     0,    53,     0,    50,     0,    13,    36,
    51,    37,     0,     0,    51,    52,    38,     0,    61,     0,
    11,     3,     0,    55,     0,    56,     0,    57,     0,    58,
     0,    59,     0,    24,     0,    25,     0,    26,     0,    27,
     0,    28,     0,    54,     3,     0,    54,     3,    30,    83,
     0,    54,     3,    39,    87,     0,    54,     3,     0,    54,
     3,    40,    62,     0,     3,     0,     3,    40,    62,     0,
    64,    36,    68,    37,     0,    54,     3,    41,    65,    42,
     0,    54,     0,    59,     0,     0,    66,     0,    67,     0,
    66,    40,    67,     0,    60,     0,     0,    68,    69,     0,
    78,     0,    79,     0,    61,    38,     0,    77,    38,     0,
    72,    38,     0,    71,     0,    70,    38,     0,    38,     0,
    12,     0,    12,    87,     0,    17,    87,    79,     0,     3,
    43,     3,    41,    75,    73,    42,     0,     3,    29,     3,
    41,    75,    73,    42,     0,     3,    41,    73,    42,     0,
     0,    74,     0,    87,     0,    74,    40,    87,     0,     0,
    75,    76,    38,     0,    44,     3,    39,    82,     0,     3,
    39,    87,     0,    14,    41,    87,    42,    79,    16,    79,
     0,    14,    41,    87,    42,    79,    16,    78,     0,    14,
    41,    87,    42,    79,     0,    36,    68,    37,     0,    86,
     0,    85,     0,    81,     0,    82,     0,     6,     0,    84,
     0,     4,     0,     5,     0,     7,     0,     8,     0,    84,
     0,     4,     0,     5,     0,    80,     0,     3,     0,     3,
    43,     3,     0,    72,     0,    87,    18,    87,     0,    87,
    19,    87,     0,    87,    21,    87,     0,    87,    20,    87,
     0,    87,    45,    87,     0,    87,    46,    87,     0,    87,
    33,    87,     0,    87,    34,    87,     0,    87,    31,    87,
     0,    87,    32,    87,     0,    87,    22,    87,     0,    87,
    23,    87,     0,    35,    87,     0,    41,    87,    42,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    51,    55,    56,    59,    59,    59,    59,    61,    64,    65,
    68,    69,    72,    72,    73,    73,    74,    76,    77,    78,
    79,    80,    81,    86,    89,    92,    96,   114,   115,   119,
   122,   125,   126,   128,   129,   131,   133,   137,   141,   143,
   146,   147,   148,   149,   150,   151,   152,   153,   155,   158,
   162,   165,   174,   177,   180,   181,   183,   184,   188,   189,
   192,   197,   200,   203,   206,   209,   212,   212,   213,   214,
   217,   219,   220,   221,   223,   226,   229,   233,   236,   239,
   240,   243,   246,   247,   249,   251,   253,   255,   257,   260,
   262,   264,   266,   268,   270,   272,   274
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","L_ID","L_FLOATCONST",
"L_INTCONST","L_STRINGCONST","L_BOOLCONST_TRUE","L_BOOLCONST_FALSE","L_MODULE",
"L_SCRIPT","L_IMPORT","L_RETURN","L_GLOBALS","L_IF","L_THEN","L_ELSE","L_WHILE",
"L_EQUAL","L_NOT_EQUAL","L_GREATER_OR_EQUAL","L_LESSER_OR_EQUAL","L_BOOL_AND",
"L_BOOL_OR","L_INT","L_FLOAT","L_BOOL","L_OBJECT","L_VOID","L_METHODCALL","L_INITVALUE",
"'-'","'+'","'*'","'/'","'!'","'{'","'}'","';'","'='","','","'('","')'","'.'",
"':'","'<'","'>'","module","module_body","module_statement","globals","globals_body",
"global_statement","import","vartype","inttype","floattype","booltype","objecttype",
"voidtype","argvar","defvar","nonnull_idlist","script","script_header","arguments",
"nonnull_arguments","argument","script_body","script_statement","return_statement",
"while_statement","call_void","call_arglist","nonnull_arglist","attributes",
"attribute","setvar","if_statement","block_statement","constant","stringconst",
"string_constant","init_val","boolvalue","boolconst","number","expr", NULL
};
#endif

static const short yyr1[] = {     0,
    47,    48,    48,    49,    49,    49,    49,    50,    51,    51,
    52,    53,    54,    54,    54,    54,    54,    55,    56,    57,
    58,    59,    60,    61,    61,    61,    61,    62,    62,    63,
    64,    -1,    -1,    65,    65,    66,    66,    67,    68,    68,
    69,    69,    69,    69,    69,    69,    69,    69,    70,    70,
    71,    72,    72,    72,    73,    73,    74,    74,    75,    75,
    76,    77,    78,    78,    78,    79,    80,    80,    80,    81,
    82,    83,    83,    83,    84,    84,    85,    86,    86,    87,
    87,    87,    87,    87,    87,    87,    87,    87,    87,    87,
    87,    87,    87,    87,    87,    87,    87
};

static const short yyr2[] = {     0,
     5,     0,     3,     1,     1,     1,     1,     4,     0,     3,
     1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     2,     4,     4,     2,     4,     1,     3,     4,
     5,     1,     1,     0,     1,     1,     3,     1,     0,     2,
     1,     1,     2,     2,     2,     1,     2,     1,     1,     2,
     3,     7,     7,     4,     0,     1,     1,     3,     0,     3,
     4,     3,     7,     7,     5,     3,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     3,     1,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     3,     3,     2,     3
};

static const short yydefact[] = {     0,
     0,     0,     2,     0,     0,     0,    18,    19,    20,    21,
    22,     1,     0,     7,     6,     0,    13,    14,    15,    16,
    17,     5,     4,     0,    12,     9,     3,    26,    39,     0,
     0,     0,     0,    34,     0,     8,     0,     0,    11,    73,
    74,    75,    76,    24,    72,    81,    78,    79,    71,     0,
     0,    83,    80,    69,    70,    77,    68,    67,    25,    28,
    27,     0,    38,     0,    35,    36,     0,    49,     0,     0,
    39,    30,    48,     0,    40,     0,    46,     0,     0,    41,
    42,    10,    26,     0,    55,     0,    96,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,    23,    31,     0,     0,     0,    50,     0,     0,     0,
    43,    47,    45,    44,     0,     0,    56,    57,    82,    97,
    84,    85,    87,    86,    94,    95,    92,    93,    90,    91,
    88,    89,    29,    37,    62,     0,     0,    51,    66,    59,
    54,     0,    59,     0,    55,    58,    55,    65,     0,     0,
     0,     0,     0,     0,    53,    60,    52,    64,    63,     0,
    61,     0,     0,     0
};

static const short yydefgoto[] = {   162,
     4,    13,    14,    30,    37,    15,    38,    17,    18,    19,
    20,    21,    63,    74,    61,    23,    24,    64,    65,    66,
    35,    75,    76,    77,    52,   116,   117,   145,   151,    79,
    80,    81,    53,    54,    55,    44,    56,    57,    58,   118
};

static const short yypact[] = {    -6,
     8,    -1,-32768,    61,    46,    20,-32768,-32768,-32768,-32768,
-32768,-32768,    13,-32768,-32768,    68,-32768,-32768,-32768,-32768,
-32768,-32768,-32768,    39,-32768,-32768,-32768,    -7,-32768,   190,
   218,     9,    76,    88,    66,-32768,    43,    92,-32768,-32768,
-32768,-32768,-32768,-32768,-32768,    41,-32768,-32768,-32768,     9,
     9,-32768,-32768,-32768,-32768,-32768,-32768,-32768,   158,    56,
-32768,    96,-32768,    58,    65,-32768,   170,     9,    93,     9,
-32768,-32768,-32768,    69,-32768,    71,-32768,    79,    91,-32768,
-32768,-32768,    15,   130,     9,   132,   175,   105,     9,     9,
     9,     9,     9,     9,     9,     9,     9,     9,     9,     9,
    76,-32768,-32768,    88,     9,   143,   158,     9,   122,    94,
-32768,-32768,-32768,-32768,   107,   110,   109,   158,   116,-32768,
   158,   158,   158,   158,   158,   158,     7,     7,   175,   175,
   158,   158,-32768,-32768,   158,   116,   141,-32768,-32768,-32768,
-32768,     9,-32768,   129,     2,   158,     2,   150,   166,   128,
   133,   140,     6,   145,-32768,-32768,-32768,-32768,-32768,   179,
-32768,   188,   200,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    -3,-32768,-32768,-32768,
-32768,-32768,-32768,    17,   100,-32768,-32768,-32768,-32768,    98,
   134,-32768,-32768,-32768,   -33,  -123,-32768,    63,-32768,-32768,
    54,  -105,-32768,-32768,    48,-32768,   181,-32768,-32768,   -32
};


#define	YYLAST		227


static const short yytable[] = {    59,
    16,    78,     1,   138,    46,    47,    48,    49,    42,    43,
     2,    46,    47,    48,    49,    42,    43,    87,    88,    69,
    22,   150,    31,   152,    89,    90,    91,    92,    93,    94,
    62,    32,    33,    34,     3,   107,    50,   109,   148,    97,
    98,    71,    51,    50,    31,   149,    39,   159,    25,    51,
    27,    99,   100,    32,    33,    26,   121,   122,   123,   124,
   125,   126,   127,   128,   129,   130,   131,   132,    67,    84,
    28,     5,   135,     6,    29,   137,    78,    68,    60,    69,
    82,    85,    70,    86,     7,     8,     9,    10,    11,     7,
     8,     9,    10,    11,    83,   101,    67,    12,   102,   103,
    62,    71,    72,    73,   104,    68,   111,    69,   112,   146,
    70,     7,     8,     9,    10,    11,   113,     7,     8,     9,
    10,    11,    89,    90,    91,    92,    93,    94,   114,    71,
   139,    73,   115,   108,   119,    95,    96,    97,    98,    89,
    90,    91,    92,    93,    94,   136,   120,   140,   142,    99,
   100,   141,    95,    96,    97,    98,   143,    71,    89,    90,
    91,    92,    93,    94,    71,   153,    99,   100,   154,   155,
   156,    95,    96,    97,    98,    89,    90,    91,    92,    93,
    94,   157,   144,   160,    49,    99,   100,   163,    95,    96,
    97,    98,    89,    90,    91,    92,    93,    94,    84,   164,
   133,   134,    99,   100,   110,   147,   158,   161,   105,     0,
    85,    45,   106,     7,     8,     9,    10,    11,     0,    99,
   100,    40,    41,     0,    42,    43,    36
};

static const short yycheck[] = {    32,
     4,    35,     9,   109,     3,     4,     5,     6,     7,     8,
     3,     3,     4,     5,     6,     7,     8,    50,    51,    14,
     4,   145,    30,   147,    18,    19,    20,    21,    22,    23,
    34,    39,    40,    41,    36,    68,    35,    70,   144,    33,
    34,    36,    41,    35,    30,    44,    30,   153,     3,    41,
    38,    45,    46,    39,    40,    36,    89,    90,    91,    92,
    93,    94,    95,    96,    97,    98,    99,   100,     3,    29,
     3,    11,   105,    13,    36,   108,   110,    12,     3,    14,
    38,    41,    17,    43,    24,    25,    26,    27,    28,    24,
    25,    26,    27,    28,     3,    40,     3,    37,     3,    42,
   104,    36,    37,    38,    40,    12,    38,    14,    38,   142,
    17,    24,    25,    26,    27,    28,    38,    24,    25,    26,
    27,    28,    18,    19,    20,    21,    22,    23,    38,    36,
    37,    38,     3,    41,     3,    31,    32,    33,    34,    18,
    19,    20,    21,    22,    23,     3,    42,    41,    40,    45,
    46,    42,    31,    32,    33,    34,    41,    36,    18,    19,
    20,    21,    22,    23,    36,    16,    45,    46,     3,    42,
    38,    31,    32,    33,    34,    18,    19,    20,    21,    22,
    23,    42,    42,    39,     6,    45,    46,     0,    31,    32,
    33,    34,    18,    19,    20,    21,    22,    23,    29,     0,
   101,   104,    45,    46,    71,   143,   153,   160,    39,    -1,
    41,    31,    43,    24,    25,    26,    27,    28,    -1,    45,
    46,     4,     5,    -1,     7,     8,    37
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
#line 51 "c_alike.yacc"
{
	string module="<module name=" + q(yyvsp[-3]) + " >\n"+yyvsp[-1]+"\n</module>\n";
	module_string=module;
;
    break;}
case 2:
#line 55 "c_alike.yacc"
{ yyval=""; ;
    break;}
case 3:
#line 56 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"\n"+yyvsp[-1];
;
    break;}
case 4:
#line 59 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 5:
#line 59 "c_alike.yacc"
{yyval=yyvsp[0];
    break;}
case 6:
#line 59 "c_alike.yacc"
{ yyval=yyvsp[0] ;
    break;}
case 7:
#line 59 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 8:
#line 61 "c_alike.yacc"
{
	yyval="<globals>\n"+yyvsp[-1]+"\n</globals>\n";
;
    break;}
case 9:
#line 64 "c_alike.yacc"
{ yyval=""; ;
    break;}
case 10:
#line 65 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"\n"+yyvsp[-1];
;
    break;}
case 11:
#line 68 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 12:
#line 69 "c_alike.yacc"
{
	yyval="<import name="+q(yyvsp[0])+"/>";
 ;
    break;}
case 13:
#line 72 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 14:
#line 72 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 15:
#line 73 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 16:
#line 73 "c_alike.yacc"
{yyval=yyvsp[0];
    break;}
case 17:
#line 74 "c_alike.yacc"
{ yyval=yyvsp[0];;
    break;}
case 18:
#line 76 "c_alike.yacc"
{ yyval="int" ;
    break;}
case 19:
#line 77 "c_alike.yacc"
{ yyval="float";
    break;}
case 20:
#line 78 "c_alike.yacc"
{  yyval="bool";
    break;}
case 21:
#line 79 "c_alike.yacc"
{ yyval="object";
    break;}
case 22:
#line 80 "c_alike.yacc"
{ yyval="void";
    break;}
case 23:
#line 81 "c_alike.yacc"
{
	yyval="<defvar name="+q(yyvsp[0])+" type="+q(yyvsp[-1])+"/>\n";
//	printf("DEVFAR %s\n",$2.c_str());
;
    break;}
case 24:
#line 86 "c_alike.yacc"
{
	yyval="<defvar name="+q(yyvsp[-2])+" type="+q(yyvsp[-3])+" initvalue="+q(yyvsp[0])+" />\n";
;
    break;}
case 25:
#line 89 "c_alike.yacc"
{
	yyval="<defvar name="+q(yyvsp[-2])+" type="+q(yyvsp[-3])+" />\n"+"<setvar name="+q(yyvsp[-2])+" >\n"+yyvsp[0]+"\n</setvar>";
;
    break;}
case 26:
#line 92 "c_alike.yacc"
{
	yyval="<defvar name="+q(yyvsp[0])+" type="+q(yyvsp[-1])+"/>\n";
//	printf("DEVFAR %s\n",$2.c_str());
;
    break;}
case 27:
#line 96 "c_alike.yacc"
{

	string allvars="";

	string list=yyvsp[-2]+" "+yyvsp[0]+" ";
	int apos=0;
	int npos=0;
	//printf("names=%s\n",list.c_str());
	do{
		 npos=list.find(" ",apos);
		string news=list.substr(apos,npos-apos);
		//printf("NEW: %s apos=%d npos=%d\n",news.c_str(),apos,npos);
		allvars=allvars+"<defvar name="+q(news)+" type="+q(yyvsp[-3])+" />\n";
		apos=npos+1;
	}while(apos<list.size()-1);
	
	yyval=allvars;
;
    break;}
case 28:
#line 114 "c_alike.yacc"
{ yyval=yyvsp[0];;
    break;}
case 29:
#line 115 "c_alike.yacc"
{
	yyval=yyvsp[-2]+" "+yyvsp[0];
;
    break;}
case 30:
#line 119 "c_alike.yacc"
{
	yyval=yyvsp[-3]+"\n"+yyvsp[-1]+"\n</script>\n";
;
    break;}
case 31:
#line 122 "c_alike.yacc"
{
	yyval="<script name="+q(yyvsp[-3])+" return="+q(yyvsp[-4])+" >\n"+"<arguments>\n"+yyvsp[-1]+"\n</arguments>\n";
;
    break;}
case 32:
#line 125 "c_alike.yacc"
{ printf("var_or_voidtype\n"); yyval=yyvsp[0];;
    break;}
case 33:
#line 126 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 34:
#line 128 "c_alike.yacc"
{ yyval="\n"; ;
    break;}
case 35:
#line 129 "c_alike.yacc"
{ yyval=yyvsp[0];;
    break;}
case 36:
#line 132 "c_alike.yacc"
{ yyval=yyvsp[0];;
    break;}
case 37:
#line 133 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"\n"+yyvsp[0];
;
    break;}
case 38:
#line 137 "c_alike.yacc"
{
	yyval=yyvsp[0];
;
    break;}
case 39:
#line 142 "c_alike.yacc"
{ yyval=""; ;
    break;}
case 40:
#line 143 "c_alike.yacc"
{
	yyval=yyvsp[-1]+"\n"+yyvsp[0];
;
    break;}
case 41:
#line 146 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 42:
#line 147 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 43:
#line 148 "c_alike.yacc"
{ yyval=yyvsp[-1]; ;
    break;}
case 44:
#line 149 "c_alike.yacc"
{ yyval=yyvsp[-1]; ;
    break;}
case 45:
#line 150 "c_alike.yacc"
{ yyval=yyvsp[-1]; ;
    break;}
case 46:
#line 151 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 47:
#line 152 "c_alike.yacc"
{ yyval=yyvsp[-1]; ;
    break;}
case 48:
#line 153 "c_alike.yacc"
{ yyval=" ";;
    break;}
case 49:
#line 155 "c_alike.yacc"
{
	yyval="<return/>\n";
;
    break;}
case 50:
#line 158 "c_alike.yacc"
{
	yyval="<return>\n"+yyvsp[0]+"\n</return>\n";
;
    break;}
case 51:
#line 162 "c_alike.yacc"
{
	yyval="<while>\n"+yyvsp[-1]+"\n"+yyvsp[0]+"\n</while>\n";
;
    break;}
case 52:
#line 165 "c_alike.yacc"
{
	if(yyvsp[-6][0]=='_'){
		yyval="<call module="+q(yyvsp[-6])+" name="+q(yyvsp[-4])+" "+yyvsp[-2]+" >\n"+yyvsp[-1]+"\n</call>\n";
	}
	else{	
		yyval="<exec module="+q(yyvsp[-6])+" name="+q(yyvsp[-4])+" "+yyvsp[-2]+" >\n"+yyvsp[-1]+"\n</exec>\n";
	}
;
    break;}
case 53:
#line 174 "c_alike.yacc"
{
		yyval="<call object="+q(yyvsp[-6])+" name="+q(yyvsp[-4])+" "+yyvsp[-2]+" >\n"+yyvsp[-1]+"\n</call>\n";
;
    break;}
case 54:
#line 177 "c_alike.yacc"
{
	yyval="<exec name="+q(yyvsp[-3])+" >\n"+yyvsp[-1]+"\n</exec>\n";
;
    break;}
case 55:
#line 180 "c_alike.yacc"
{ yyval="\n"; ;
    break;}
case 56:
#line 181 "c_alike.yacc"
{ yyval=yyvsp[0];;
    break;}
case 57:
#line 183 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 58:
#line 184 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"\n"+yyvsp[0];
;
    break;}
case 59:
#line 188 "c_alike.yacc"
{ yyval=" "; ;
    break;}
case 60:
#line 189 "c_alike.yacc"
{
	yyval=yyvsp[-2]+" "+yyvsp[-1];
;
    break;}
case 61:
#line 192 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"="+yyvsp[0]+" ";
;
    break;}
case 62:
#line 197 "c_alike.yacc"
{
	yyval="<setvar name="+q(yyvsp[-2])+" >\n"+yyvsp[0]+"\n</setvar>\n";
;
    break;}
case 63:
#line 200 "c_alike.yacc"
{
	yyval="<if>\n"+yyvsp[-4]+"\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</if>\n";
;
    break;}
case 64:
#line 203 "c_alike.yacc"
{
	yyval="<if>\n"+yyvsp[-4]+"\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</if>\n";
;
    break;}
case 65:
#line 206 "c_alike.yacc"
{
	yyval="<if>\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n"+"<block></block>"+"\n</if>\n";
;
    break;}
case 66:
#line 209 "c_alike.yacc"
{
	yyval="<block>\n"+yyvsp[-1]+"\n</block>\n";
;
    break;}
case 67:
#line 212 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 68:
#line 212 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 69:
#line 213 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 70:
#line 214 "c_alike.yacc"
{
	yyval="<const type=\"object\" object=\"string\" value="+yyvsp[0]+" />\n";
;
    break;}
case 71:
#line 217 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 72:
#line 219 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 73:
#line 220 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 74:
#line 221 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 75:
#line 223 "c_alike.yacc"
{
	 yyval="true";
 ;
    break;}
case 76:
#line 226 "c_alike.yacc"
{
	 yyval="false";
;
    break;}
case 77:
#line 229 "c_alike.yacc"
{
	 yyval="<const type=\"bool\" value="+q(yyvsp[0])+" />\n";
 ;
    break;}
case 78:
#line 233 "c_alike.yacc"
{
	 yyval="<const type=\"float\" value="+q(yyvsp[0])+" />\n";
 ;
    break;}
case 79:
#line 236 "c_alike.yacc"
{
	 yyval="<const type=\"int\" value="+q(yyvsp[0])+" />\n";
;
    break;}
case 80:
#line 239 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 81:
#line 240 "c_alike.yacc"
{
	yyval="<var name="+q(yyvsp[0])+" />\n";
;
    break;}
case 82:
#line 243 "c_alike.yacc"
{
	yyval="<var module="+q(yyvsp[-2])+" name="+q(yyvsp[-1])+" />\n";
;
    break;}
case 83:
#line 246 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 84:
#line 248 "c_alike.yacc"
{ yyval="<test test=\"eq\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 85:
#line 250 "c_alike.yacc"
{ yyval="<test test=\"ne\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 86:
#line 252 "c_alike.yacc"
{ yyval="<test test=\"le\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 87:
#line 254 "c_alike.yacc"
{ yyval="<test test=\"ge\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 88:
#line 256 "c_alike.yacc"
{ yyval="<test test=\"lt\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 89:
#line 258 "c_alike.yacc"
{ yyval="<test test=\"gt\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 90:
#line 261 "c_alike.yacc"
{ yyval="<fmath math=\"*\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</fmath>\n"; ;
    break;}
case 91:
#line 263 "c_alike.yacc"
{ yyval="<fmath math=\"/\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</fmath>\n"; ;
    break;}
case 92:
#line 265 "c_alike.yacc"
{ yyval="<fmath math=\"-\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</fmath>\n"; ;
    break;}
case 93:
#line 267 "c_alike.yacc"
{ yyval="<fmath math=\"+\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</fmath>\n"; ;
    break;}
case 94:
#line 269 "c_alike.yacc"
{ yyval="<and>\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</and>\n"; ;
    break;}
case 95:
#line 271 "c_alike.yacc"
{ yyval="<or>\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</or>\n"; ;
    break;}
case 96:
#line 273 "c_alike.yacc"
{ yyval="<not>\n"+yyvsp[0]+"\n</not>\n";		;
    break;}
case 97:
#line 275 "c_alike.yacc"
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
#line 277 "c_alike.yacc"


/* nothing */


/* extern int yydebug; */

string parseCalike(char const *filename)
{
  string empty_string;

  yylineno=0;

  yyin=fopen(filename,"r");

  if(yyin==NULL){
    // file not found
    return empty_string;
  }

/*  yydebug=1; */
  yyparse();
  
  return module_string;
}


int yyerror(char *s){
  printf("(yy)error: %s line %d text -%s-\n",s,yylineno,yytext);
  return 1;
}

int yywrap(){
	return 1;
}


