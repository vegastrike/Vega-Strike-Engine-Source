
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
#define	L_CLASS	268
#define	L_IF	269
#define	L_THEN	270
#define	L_ELSE	271
#define	L_WHILE	272
#define	L_EQUAL	273
#define	L_NOT_EQUAL	274
#define	L_GREATER_OR_EQUAL	275
#define	L_LESSER_OR_EQUAL	276
#define	L_BOOL_AND	277
#define	L_BOOL_OR	278
#define	L_INT	279
#define	L_FLOAT	280
#define	L_BOOL	281
#define	L_OBJECT	282
#define	L_VOID	283
#define	L_METHODCALL	284
#define	L_INITVALUE	285

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

std::string module_string;

bool have_yy_error;

//string module_string;

string lineno(){
char buffer[100];

	sprintf(buffer," line=\"%d\" ",yylineno+1);

	return buffer;
}

#ifndef YYSTYPE
#define YYSTYPE int
#endif
#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		167
#define	YYFLAG		-32768
#define	YYNTBASE	48

#define YYTRANSLATE(x) ((unsigned)(x) <= 285 ? yytranslate[x] : 90)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    36,     2,     2,     2,     2,     2,     2,    42,
    43,    34,    33,    41,    32,    44,    35,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    45,    39,    46,
    40,    47,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,    37,     2,    38,     2,     2,     2,     2,     2,
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
    50,    52,    54,    57,    59,    60,    62,    65,    70,    75,
    80,    83,    89,    94,    97,   102,   104,   108,   113,   119,
   121,   123,   124,   126,   128,   132,   134,   135,   138,   140,
   142,   145,   148,   151,   153,   156,   158,   160,   163,   167,
   175,   183,   188,   189,   191,   193,   197,   198,   202,   207,
   211,   219,   227,   233,   237,   239,   241,   243,   245,   247,
   249,   251,   253,   255,   257,   259,   261,   263,   265,   267,
   271,   273,   277,   281,   285,   289,   293,   297,   301,   305,
   309,   313,   317,   321,   324
};

static const short yyrhs[] = {     9,
     3,    37,    49,    38,     0,     0,    49,    50,    39,     0,
    63,     0,    65,     0,    54,     0,    51,     0,    13,    37,
    52,    38,     0,     0,    52,    53,    39,     0,    63,     0,
    11,     3,     0,    56,     0,    57,     0,    58,     0,    59,
     0,    60,     0,    25,     0,    26,     0,    27,     0,    28,
     0,    29,     0,    55,     3,     0,    14,     0,     0,    55,
     0,    14,    55,     0,    62,     3,    40,    89,     0,    62,
     3,    31,    85,     0,    62,     3,    41,    64,     0,    62,
     3,     0,     0,    55,     3,    31,    85,     0,    55,     3,
    40,    89,     0,    55,     3,     0,    55,     3,    41,    64,
     0,     3,     0,     3,    41,    64,     0,    66,    37,    70,
    38,     0,    62,     3,    42,    67,    43,     0,    55,     0,
    60,     0,     0,    68,     0,    69,     0,    68,    41,    69,
     0,    61,     0,     0,    70,    71,     0,    80,     0,    81,
     0,    63,    39,     0,    79,    39,     0,    74,    39,     0,
    73,     0,    72,    39,     0,    39,     0,    12,     0,    12,
    89,     0,    18,    89,    81,     0,     3,    44,     3,    42,
    77,    75,    43,     0,     3,    30,     3,    42,    77,    75,
    43,     0,     3,    42,    75,    43,     0,     0,    76,     0,
    89,     0,    76,    41,    89,     0,     0,    77,    78,    39,
     0,    45,     3,    40,    84,     0,     3,    40,    89,     0,
    15,    42,    89,    43,    81,    17,    81,     0,    15,    42,
    89,    43,    81,    17,    80,     0,    15,    42,    89,    43,
    81,     0,    37,    70,    38,     0,    88,     0,    87,     0,
    83,     0,    84,     0,     6,     0,    86,     0,     4,     0,
     5,     0,     7,     0,     8,     0,    86,     0,     4,     0,
     5,     0,    82,     0,     3,     0,     3,    44,     3,     0,
    74,     0,    89,    19,    89,     0,    89,    20,    89,     0,
    89,    22,    89,     0,    89,    21,    89,     0,    89,    46,
    89,     0,    89,    47,    89,     0,    89,    34,    89,     0,
    89,    35,    89,     0,    89,    32,    89,     0,    89,    33,
    89,     0,    89,    23,    89,     0,    89,    24,    89,     0,
    36,    89,     0,    42,    89,    43,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    63,    67,    68,    71,    71,    71,    71,    73,    76,    77,
    80,    81,    84,    84,    85,    85,    86,    88,    89,    90,
    91,    92,    93,    97,   100,   104,   108,   112,   115,   118,
   136,   141,   144,   147,   151,   169,   170,   174,   177,   181,
   182,   184,   185,   187,   189,   193,   197,   199,   202,   203,
   204,   205,   206,   207,   208,   209,   211,   214,   218,   221,
   230,   233,   236,   237,   239,   240,   244,   245,   248,   253,
   256,   259,   262,   265,   268,   268,   269,   270,   273,   275,
   276,   277,   279,   282,   285,   289,   292,   295,   296,   299,
   302,   303,   305,   307,   309,   311,   313,   316,   318,   320,
   322,   324,   326,   328,   330
};
#endif


#if YYDEBUG != 0 || defined (YYERROR_VERBOSE)

static const char * const yytname[] = {   "$","error","$undefined.","L_ID","L_FLOATCONST",
"L_INTCONST","L_STRINGCONST","L_BOOLCONST_TRUE","L_BOOLCONST_FALSE","L_MODULE",
"L_SCRIPT","L_IMPORT","L_RETURN","L_GLOBALS","L_CLASS","L_IF","L_THEN","L_ELSE",
"L_WHILE","L_EQUAL","L_NOT_EQUAL","L_GREATER_OR_EQUAL","L_LESSER_OR_EQUAL","L_BOOL_AND",
"L_BOOL_OR","L_INT","L_FLOAT","L_BOOL","L_OBJECT","L_VOID","L_METHODCALL","L_INITVALUE",
"'-'","'+'","'*'","'/'","'!'","'{'","'}'","';'","'='","','","'('","')'","'.'",
"':'","'<'","'>'","module","module_body","module_statement","globals","globals_body",
"global_statement","import","vartype","inttype","floattype","booltype","objecttype",
"voidtype","argvar","defvar_begin","defvar","nonnull_idlist","script","script_header",
"arguments","nonnull_arguments","argument","script_body","script_statement",
"return_statement","while_statement","call_void","call_arglist","nonnull_arglist",
"attributes","attribute","setvar","if_statement","block_statement","constant",
"stringconst","string_constant","init_val","boolvalue","boolconst","number",
"expr", NULL
};
#endif

static const short yyr1[] = {     0,
    48,    49,    49,    50,    50,    50,    50,    51,    52,    52,
    53,    54,    55,    55,    55,    55,    55,    56,    57,    58,
    59,    60,    61,    -1,    -1,    62,    62,    63,    63,    63,
    63,    -1,    -1,    -1,    -1,    64,    64,    65,    66,    -1,
    -1,    67,    67,    68,    68,    69,    70,    70,    71,    71,
    71,    71,    71,    71,    71,    71,    72,    72,    73,    74,
    74,    74,    75,    75,    76,    76,    77,    77,    78,    79,
    80,    80,    80,    81,    82,    82,    82,    83,    84,    85,
    85,    85,    86,    86,    87,    88,    88,    89,    89,    89,
    89,    89,    89,    89,    89,    89,    89,    89,    89,    89,
    89,    89,    89,    89,    89
};

static const short yyr2[] = {     0,
     5,     0,     3,     1,     1,     1,     1,     4,     0,     3,
     1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
     1,     1,     2,     1,     0,     1,     2,     4,     4,     4,
     2,     5,     4,     2,     4,     1,     3,     4,     5,     1,
     1,     0,     1,     1,     3,     1,     0,     2,     1,     1,
     2,     2,     2,     1,     2,     1,     1,     2,     3,     7,
     7,     4,     0,     1,     1,     3,     0,     3,     4,     3,
     7,     7,     5,     3,     1,     1,     1,     1,     1,     1,
     1,     1,     1,     1,     1,     1,     1,     1,     1,     3,
     1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
     3,     3,     3,     2,     3
};

static const short yydefact[] = {     0,
     0,     0,     2,     0,     0,     0,     0,    18,    19,    20,
    21,    22,     1,     0,     7,     6,    26,    13,    14,    15,
    16,    17,     0,     4,     5,     0,    12,     9,    27,     3,
    31,    47,     0,     0,     0,     0,    42,     0,     8,     0,
     0,    11,    81,    82,    83,    84,    29,    80,    89,    86,
    87,    79,     0,     0,    91,    88,    77,    78,    85,    76,
    75,    28,    36,    30,     0,    46,     0,    43,    44,     0,
    57,     0,     0,    47,    38,    56,     0,    48,     0,    54,
     0,     0,    49,    50,    10,    31,     0,    63,     0,   104,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    23,    39,     0,     0,     0,    58,
     0,     0,     0,    51,    55,    53,    52,     0,     0,    64,
    65,    90,   105,    92,    93,    95,    94,   102,   103,   100,
   101,    98,    99,    96,    97,    37,    45,    70,     0,     0,
    59,    74,    67,    62,     0,    67,     0,    63,    66,    63,
    73,     0,     0,     0,     0,     0,     0,    61,    68,    60,
    72,    71,     0,    69,     0,     0,     0
};

static const short yydefgoto[] = {   165,
     4,    14,    15,    33,    40,    16,    17,    18,    19,    20,
    21,    22,    66,    41,    77,    64,    25,    26,    67,    68,
    69,    38,    78,    79,    80,    55,   119,   120,   148,   154,
    82,    83,    84,    56,    57,    58,    47,    59,    60,    61,
   121
};

static const short yypact[] = {    11,
    22,    -8,-32768,   206,    27,    -3,    16,-32768,-32768,-32768,
-32768,-32768,-32768,    -2,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,-32768,    44,-32768,-32768,    19,-32768,-32768,-32768,-32768,
   -25,-32768,   222,    67,    47,    76,    16,    66,-32768,    31,
    79,-32768,-32768,-32768,-32768,-32768,-32768,-32768,    -9,-32768,
-32768,-32768,    47,    47,-32768,-32768,-32768,-32768,-32768,-32768,
-32768,   158,    45,-32768,    85,-32768,    53,    49,-32768,   -16,
    47,    56,    47,-32768,-32768,-32768,    60,-32768,    61,-32768,
    63,    68,-32768,-32768,-32768,   -18,   108,    47,   110,   192,
   105,    47,    47,    47,    47,    47,    47,    47,    47,    47,
    47,    47,    47,    76,-32768,-32768,    16,    47,   111,   158,
    47,   122,    94,-32768,-32768,-32768,-32768,    73,    74,    75,
   158,    88,-32768,   158,   158,   158,   158,   158,   158,   175,
   175,   192,   192,   158,   158,-32768,-32768,   158,    88,   141,
-32768,-32768,-32768,-32768,    47,-32768,    81,     4,   158,     4,
   117,   132,    93,   114,   104,   -10,   109,-32768,-32768,-32768,
-32768,-32768,   144,-32768,   166,   167,-32768
};

static const short yypgoto[] = {-32768,
-32768,-32768,-32768,-32768,-32768,-32768,    -6,-32768,-32768,-32768,
-32768,-32768,-32768,   154,    -1,    82,-32768,-32768,-32768,-32768,
    64,    96,-32768,-32768,-32768,   -36,   -63,-32768,    26,-32768,
-32768,    29,  -108,-32768,-32768,    20,-32768,   155,-32768,-32768,
   -35
};


#define	YYLAST		260


static const short yytable[] = {    62,
    29,    81,    24,   141,    72,    34,    49,    50,    51,    52,
    45,    46,    34,    87,    35,    36,    37,    90,    91,     1,
    87,    35,    36,   108,     2,    88,    74,   109,     3,    27,
    65,    42,    88,    28,    89,   110,    30,   112,   151,    53,
     8,     9,    10,    11,    12,    54,    31,   162,   152,    49,
    50,    51,    52,    45,    46,    32,   124,   125,   126,   127,
   128,   129,   130,   131,   132,   133,   134,   135,    70,    85,
    43,    44,   138,    45,    46,   140,    81,    71,    63,     7,
    72,    86,    53,    73,   153,   104,   155,   105,    54,   107,
     8,     9,    10,    11,    12,   106,    70,   111,   114,   115,
    65,   116,    74,    75,    76,    71,   117,     7,    72,   149,
   118,    73,   122,   139,   143,   145,   144,    74,     8,     9,
    10,    11,    12,    92,    93,    94,    95,    96,    97,   146,
    74,   142,    76,   156,   157,   158,    98,    99,   100,   101,
    92,    93,    94,    95,    96,    97,   160,   123,   163,    52,
   102,   103,   159,    98,    99,   100,   101,    23,    74,    92,
    93,    94,    95,    96,    97,   166,   167,   102,   103,   113,
   137,   150,    98,    99,   100,   101,    92,    93,    94,    95,
    96,    97,   164,   147,   161,   136,   102,   103,    48,    98,
    99,   100,   101,    92,    93,    94,    95,    96,    97,     0,
     0,     0,     0,   102,   103,     0,     0,     0,   100,   101,
    92,    93,    94,    95,    96,    97,     5,     0,     6,     7,
   102,   103,     0,     0,     0,     0,     0,     0,     0,     0,
     8,     9,    10,    11,    12,     7,     0,   102,   103,     0,
     0,     0,     0,    13,     0,     0,     8,     9,    10,    11,
    12,     0,     0,     0,     0,     0,     0,     0,     0,    39
};

static const short yycheck[] = {    35,
     7,    38,     4,   112,    15,    31,     3,     4,     5,     6,
     7,     8,    31,    30,    40,    41,    42,    53,    54,     9,
    30,    40,    41,    40,     3,    42,    37,    44,    37,     3,
    37,    33,    42,    37,    44,    71,    39,    73,   147,    36,
    25,    26,    27,    28,    29,    42,     3,   156,    45,     3,
     4,     5,     6,     7,     8,    37,    92,    93,    94,    95,
    96,    97,    98,    99,   100,   101,   102,   103,     3,    39,
     4,     5,   108,     7,     8,   111,   113,    12,     3,    14,
    15,     3,    36,    18,   148,    41,   150,     3,    42,    41,
    25,    26,    27,    28,    29,    43,     3,    42,    39,    39,
   107,    39,    37,    38,    39,    12,    39,    14,    15,   145,
     3,    18,     3,     3,    42,    41,    43,    37,    25,    26,
    27,    28,    29,    19,    20,    21,    22,    23,    24,    42,
    37,    38,    39,    17,     3,    43,    32,    33,    34,    35,
    19,    20,    21,    22,    23,    24,    43,    43,    40,     6,
    46,    47,    39,    32,    33,    34,    35,     4,    37,    19,
    20,    21,    22,    23,    24,     0,     0,    46,    47,    74,
   107,   146,    32,    33,    34,    35,    19,    20,    21,    22,
    23,    24,   163,    43,   156,   104,    46,    47,    34,    32,
    33,    34,    35,    19,    20,    21,    22,    23,    24,    -1,
    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    34,    35,
    19,    20,    21,    22,    23,    24,    11,    -1,    13,    14,
    46,    47,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
    25,    26,    27,    28,    29,    14,    -1,    46,    47,    -1,
    -1,    -1,    -1,    38,    -1,    -1,    25,    26,    27,    28,
    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38
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
#line 63 "c_alike.yacc"
{
	string module="<module  "+lineno()+" name=" + q(yyvsp[-3]) + " >\n"+yyvsp[-1]+"\n</module>\n";
	module_string=module;
;
    break;}
case 2:
#line 67 "c_alike.yacc"
{ yyval=""; ;
    break;}
case 3:
#line 68 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"\n"+yyvsp[-1];
;
    break;}
case 4:
#line 71 "c_alike.yacc"
{yyval=yyvsp[0];
    break;}
case 5:
#line 71 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 6:
#line 71 "c_alike.yacc"
{ yyval=yyvsp[0] ;
    break;}
case 7:
#line 71 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 8:
#line 73 "c_alike.yacc"
{
	yyval="<globals "+lineno()+" >\n"+yyvsp[-1]+"\n</globals>\n";
;
    break;}
case 9:
#line 76 "c_alike.yacc"
{ yyval=""; ;
    break;}
case 10:
#line 77 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"\n"+yyvsp[-1];
;
    break;}
case 11:
#line 80 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 12:
#line 81 "c_alike.yacc"
{
	yyval="<import  "+lineno()+" name="+q(yyvsp[0])+"/>";
 ;
    break;}
case 13:
#line 84 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 14:
#line 84 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 15:
#line 85 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 16:
#line 85 "c_alike.yacc"
{yyval=yyvsp[0];
    break;}
case 17:
#line 86 "c_alike.yacc"
{ yyval=yyvsp[0];;
    break;}
case 18:
#line 88 "c_alike.yacc"
{ yyval="int" ;
    break;}
case 19:
#line 89 "c_alike.yacc"
{ yyval="float";
    break;}
case 20:
#line 90 "c_alike.yacc"
{  yyval="bool";
    break;}
case 21:
#line 91 "c_alike.yacc"
{ yyval="object";
    break;}
case 22:
#line 92 "c_alike.yacc"
{ yyval="void";
    break;}
case 23:
#line 93 "c_alike.yacc"
{
	yyval="<defvar name="+q(yyvsp[0])+" type="+q(yyvsp[-1])+"/>\n";
//	printf("DEVFAR %s\n",$2.c_str());
;
    break;}
case 24:
#line 97 "c_alike.yacc"
{
	 yyval="true";
;
    break;}
case 25:
#line 100 "c_alike.yacc"
{
	yyval="false";
;
    break;}
case 26:
#line 104 "c_alike.yacc"
{
	yyval=" classvar=\"false\" type="+q(yyvsp[0]);
;
    break;}
case 27:
#line 108 "c_alike.yacc"
{
	yyval=" classvar=\"true\" type="+q(yyvsp[0]);
;
    break;}
case 28:
#line 112 "c_alike.yacc"
{
	yyval="<defvar  "+lineno()+" name="+q(yyvsp[-2])+" "+yyvsp[-3]+" />\n"+"<setvar name="+q(yyvsp[-2])+" >\n"+yyvsp[0]+"\n</setvar>\n";
;
    break;}
case 29:
#line 115 "c_alike.yacc"
{
	yyval="<defvar  "+lineno()+" name="+q(yyvsp[-2])+" "+yyvsp[-3]+" initvalue="+q(yyvsp[0])+" />\n";
;
    break;}
case 30:
#line 118 "c_alike.yacc"
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
		//allvars=allvars+"<defvar name="+q(news)+" type="+q($1)+" />\n";
		allvars=allvars+"<defvar name="+q(news)+" "+yyvsp[-3]+" />\n";
		apos=npos+1;
	}while(apos<list.size()-1);
	
	yyval=allvars;
;
    break;}
case 31:
#line 136 "c_alike.yacc"
{
	yyval="<defvar  "+lineno()+" name="+q(yyvsp[0])+" "+yyvsp[-1]+" />\n";
;
    break;}
case 32:
#line 141 "c_alike.yacc"
{
	yyval="<defvar  "+lineno()+" name="+q(yyvsp[-2])+" type="+q(yyvsp[-3])+" initvalue="+q(yyvsp[0])+" classvar="+q(yyvsp[-4])+" />\n";
;
    break;}
case 33:
#line 144 "c_alike.yacc"
{
	yyval="<defvar  "+lineno()+" name="+q(yyvsp[-2])+" type="+q(yyvsp[-3])+" />\n"+"<setvar name="+q(yyvsp[-2])+" >\n"+yyvsp[0]+"\n</setvar>";
;
    break;}
case 34:
#line 147 "c_alike.yacc"
{
	yyval="<defvar  "+lineno()+" name="+q(yyvsp[0])+" type="+q(yyvsp[-1])+"/>\n";
	printf("DEVFAR %s\n",yyvsp[0].c_str());
;
    break;}
case 35:
#line 151 "c_alike.yacc"
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
case 36:
#line 169 "c_alike.yacc"
{ yyval=yyvsp[0];;
    break;}
case 37:
#line 170 "c_alike.yacc"
{
	yyval=yyvsp[-2]+" "+yyvsp[0];
;
    break;}
case 38:
#line 174 "c_alike.yacc"
{
	yyval=yyvsp[-3]+"\n"+yyvsp[-1]+"\n</script>\n";
;
    break;}
case 39:
#line 177 "c_alike.yacc"
{
//	$$="<script  "+lineno()+" name="+q($2)+" return="+q($1)+" >\n"+"<arguments>\n"+$4+"\n</arguments>\n";
	yyval="<script  "+lineno()+" name="+q(yyvsp[-3])+" "+yyvsp[-4]+" >\n"+"<arguments>\n"+yyvsp[-1]+"\n</arguments>\n";
;
    break;}
case 40:
#line 181 "c_alike.yacc"
{ printf("var_or_voidtype\n"); yyval=yyvsp[0];;
    break;}
case 41:
#line 182 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 42:
#line 184 "c_alike.yacc"
{ yyval="\n"; ;
    break;}
case 43:
#line 185 "c_alike.yacc"
{ yyval=yyvsp[0];;
    break;}
case 44:
#line 188 "c_alike.yacc"
{ yyval=yyvsp[0];;
    break;}
case 45:
#line 189 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"\n"+yyvsp[0];
;
    break;}
case 46:
#line 193 "c_alike.yacc"
{
	yyval=yyvsp[0];
;
    break;}
case 47:
#line 198 "c_alike.yacc"
{ yyval=""; ;
    break;}
case 48:
#line 199 "c_alike.yacc"
{
	yyval=yyvsp[-1]+"\n"+yyvsp[0];
;
    break;}
case 49:
#line 202 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 50:
#line 203 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 51:
#line 204 "c_alike.yacc"
{ yyval=yyvsp[-1]; ;
    break;}
case 52:
#line 205 "c_alike.yacc"
{ yyval=yyvsp[-1]; ;
    break;}
case 53:
#line 206 "c_alike.yacc"
{ yyval=yyvsp[-1]; ;
    break;}
case 54:
#line 207 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 55:
#line 208 "c_alike.yacc"
{ yyval=yyvsp[-1]; ;
    break;}
case 56:
#line 209 "c_alike.yacc"
{ yyval=" ";;
    break;}
case 57:
#line 211 "c_alike.yacc"
{
	yyval="<return "+lineno()+" />\n";
;
    break;}
case 58:
#line 214 "c_alike.yacc"
{
	yyval="<return "+lineno()+" >\n"+yyvsp[0]+"\n</return>\n";
;
    break;}
case 59:
#line 218 "c_alike.yacc"
{
	yyval="<while "+lineno()+" >\n"+yyvsp[-1]+"\n"+yyvsp[0]+"\n</while>\n";
;
    break;}
case 60:
#line 221 "c_alike.yacc"
{
	if(yyvsp[-6][0]=='_'){
		yyval="<call  "+lineno()+" module="+q(yyvsp[-6])+" name="+q(yyvsp[-4])+" "+yyvsp[-2]+" >\n"+yyvsp[-1]+"\n</call>\n";
	}
	else{	
		yyval="<exec  "+lineno()+" module="+q(yyvsp[-6])+" name="+q(yyvsp[-4])+" "+yyvsp[-2]+" >\n"+yyvsp[-1]+"\n</exec>\n";
	}
;
    break;}
case 61:
#line 230 "c_alike.yacc"
{
		yyval="<call  "+lineno()+" object="+q(yyvsp[-6])+" name="+q(yyvsp[-4])+" "+yyvsp[-2]+" >\n"+yyvsp[-1]+"\n</call>\n";
;
    break;}
case 62:
#line 233 "c_alike.yacc"
{
	yyval="<exec  "+lineno()+" name="+q(yyvsp[-3])+" >\n"+yyvsp[-1]+"\n</exec>\n";
;
    break;}
case 63:
#line 236 "c_alike.yacc"
{ yyval="\n"; ;
    break;}
case 64:
#line 237 "c_alike.yacc"
{ yyval=yyvsp[0];;
    break;}
case 65:
#line 239 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 66:
#line 240 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"\n"+yyvsp[0];
;
    break;}
case 67:
#line 244 "c_alike.yacc"
{ yyval=" "; ;
    break;}
case 68:
#line 245 "c_alike.yacc"
{
	yyval=yyvsp[-2]+" "+yyvsp[-1];
;
    break;}
case 69:
#line 248 "c_alike.yacc"
{
	yyval=yyvsp[-2]+"="+yyvsp[0]+" ";
;
    break;}
case 70:
#line 253 "c_alike.yacc"
{
	yyval="<setvar name="+q(yyvsp[-2])+lineno()+" >\n"+yyvsp[0]+"\n</setvar>\n";
;
    break;}
case 71:
#line 256 "c_alike.yacc"
{
	yyval="<if "+lineno()+" >\n"+yyvsp[-4]+"\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</if>\n";
;
    break;}
case 72:
#line 259 "c_alike.yacc"
{
	yyval="<if "+lineno()+" >\n"+yyvsp[-4]+"\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</if>\n";
;
    break;}
case 73:
#line 262 "c_alike.yacc"
{
	yyval="<if "+lineno()+" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n"+"<block></block>"+"\n</if>\n";
;
    break;}
case 74:
#line 265 "c_alike.yacc"
{
	yyval="<block "+lineno()+" >\n"+yyvsp[-1]+"\n</block>\n";
;
    break;}
case 75:
#line 268 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 76:
#line 268 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 77:
#line 269 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 78:
#line 270 "c_alike.yacc"
{
	yyval="<const "+lineno()+"  type=\"object\" object=\"string\" value="+yyvsp[0]+" />\n";
;
    break;}
case 79:
#line 273 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 80:
#line 275 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 81:
#line 276 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 82:
#line 277 "c_alike.yacc"
{yyval=yyvsp[0];;
    break;}
case 83:
#line 279 "c_alike.yacc"
{
	 yyval="true";
 ;
    break;}
case 84:
#line 282 "c_alike.yacc"
{
	 yyval="false";
;
    break;}
case 85:
#line 285 "c_alike.yacc"
{
	 yyval="<const  "+lineno()+" type=\"bool\" value="+q(yyvsp[0])+" />\n";
 ;
    break;}
case 86:
#line 289 "c_alike.yacc"
{
	 yyval="<const  "+lineno()+" type=\"float\" value="+q(yyvsp[0])+" />\n";
 ;
    break;}
case 87:
#line 292 "c_alike.yacc"
{
	 yyval="<const  "+lineno()+" type=\"int\" value="+q(yyvsp[0])+" />\n";
;
    break;}
case 88:
#line 295 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 89:
#line 296 "c_alike.yacc"
{
	yyval="<var  "+lineno()+" name="+q(yyvsp[0])+" />\n";
;
    break;}
case 90:
#line 299 "c_alike.yacc"
{
	yyval="<var  "+lineno()+" module="+q(yyvsp[-2])+" name="+q(yyvsp[-1])+" />\n";
;
    break;}
case 91:
#line 302 "c_alike.yacc"
{ yyval=yyvsp[0]; ;
    break;}
case 92:
#line 304 "c_alike.yacc"
{ yyval="<test  "+lineno()+" test=\"eq\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 93:
#line 306 "c_alike.yacc"
{ yyval="<test  "+lineno()+" test=\"ne\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 94:
#line 308 "c_alike.yacc"
{ yyval="<test  "+lineno()+" test=\"le\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 95:
#line 310 "c_alike.yacc"
{ yyval="<test  "+lineno()+" test=\"ge\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 96:
#line 312 "c_alike.yacc"
{ yyval="<test  "+lineno()+" test=\"lt\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 97:
#line 314 "c_alike.yacc"
{ yyval="<test  "+lineno()+" test=\"gt\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</test>\n"; ;
    break;}
case 98:
#line 317 "c_alike.yacc"
{ yyval="<fmath  "+lineno()+" math=\"*\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</fmath>\n"; ;
    break;}
case 99:
#line 319 "c_alike.yacc"
{ yyval="<fmath  "+lineno()+" math=\"/\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</fmath>\n"; ;
    break;}
case 100:
#line 321 "c_alike.yacc"
{ yyval="<fmath  "+lineno()+" math=\"-\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</fmath>\n"; ;
    break;}
case 101:
#line 323 "c_alike.yacc"
{ yyval="<fmath  "+lineno()+" math=\"+\" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</fmath>\n"; ;
    break;}
case 102:
#line 325 "c_alike.yacc"
{ yyval="<and "+lineno()+" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</and>\n"; ;
    break;}
case 103:
#line 327 "c_alike.yacc"
{ yyval="<or "+lineno()+" >\n"+yyvsp[-2]+"\n"+yyvsp[0]+"\n</or>\n"; ;
    break;}
case 104:
#line 329 "c_alike.yacc"
{ yyval="<not "+lineno()+" >\n"+yyvsp[0]+"\n</not>\n";		;
    break;}
case 105:
#line 331 "c_alike.yacc"
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
#line 333 "c_alike.yacc"


/* nothing */


/* extern int yydebug; */

string parseCalike(char const *filename)
{
  string empty_string;

  have_yy_error=false;

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
  printf("(yy)error: %s line %d text -%s-\n",s,yylineno+1,yytext);
  have_yy_error=true;

  return 1;
}

int yywrap(){
	return 1;
}


