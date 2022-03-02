/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software Foundation, Inc.

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

/* A Bison parser, made by GNU Bison 1.875.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
/* Put the tokens into the symbol table, so that GDB and other debuggers
   know about them.  */
enum yytokentype {
    L_ID = 258,
    L_FLOATCONST = 259,
    L_INTCONST = 260,
    L_STRINGCONST = 261,
    L_BOOLCONST_TRUE = 262,
    L_BOOLCONST_FALSE = 263,
    L_MODULE = 264,
    L_SCRIPT = 265,
    L_IMPORT = 266,
    L_RETURN = 267,
    L_GLOBALS = 268,
    L_CLASS = 269,
    L_IF = 270,
    L_THEN = 271,
    L_ELSE = 272,
    L_WHILE = 273,
    L_EQUAL = 274,
    L_NOT_EQUAL = 275,
    L_GREATER_OR_EQUAL = 276,
    L_LESSER_OR_EQUAL = 277,
    L_BOOL_AND = 278,
    L_BOOL_OR = 279,
    L_INT = 280,
    L_FLOAT = 281,
    L_BOOL = 282,
    L_OBJECT = 283,
    L_VOID = 284,
    L_METHODCALL = 285,
    L_INITVALUE = 286
};
#endif
#define L_ID 258
#define L_FLOATCONST 259
#define L_INTCONST 260
#define L_STRINGCONST 261
#define L_BOOLCONST_TRUE 262
#define L_BOOLCONST_FALSE 263
#define L_MODULE 264
#define L_SCRIPT 265
#define L_IMPORT 266
#define L_RETURN 267
#define L_GLOBALS 268
#define L_CLASS 269
#define L_IF 270
#define L_THEN 271
#define L_ELSE 272
#define L_WHILE 273
#define L_EQUAL 274
#define L_NOT_EQUAL 275
#define L_GREATER_OR_EQUAL 276
#define L_LESSER_OR_EQUAL 277
#define L_BOOL_AND 278
#define L_BOOL_OR 279
#define L_INT 280
#define L_FLOAT 281
#define L_BOOL 282
#define L_OBJECT 283
#define L_VOID 284
#define L_METHODCALL 285
#define L_INITVALUE 286

#if !defined (YYSTYPE) && !defined (YYSTYPE_IS_DECLARED)
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;



