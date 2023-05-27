/* 
 * Vega Strike
 * Copyright (C) 2001-2002 Daniel Horn
 * Copyright (C) 2022 Stephen G. Tuggy
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
// NO HEADER GUARD

/*
  c_alike scripting written by Alexander Rawass <alexannika@users.sourceforge.net>
*/


#include <stdio.h>

#if defined(HAVE_LINUX_SLAB_H)
#include <linux/slab.h>
#elif defined(HAVE_LINUX_MALLOC_H)
#include <linux/malloc.h>
#elif defined(__APPLE__) || defined(MACOSX)
#include <sys/malloc.h>
#elif defined(HAVE_MALLOC_H)
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include <vector>
#include <string>

using std::string;

#define YYERROR_VERBOSE

#define q(x)    ("\""+x+"\"")

extern std::string parseCalike(char const *filename);
extern int yyerror(char *);
extern int yyparse();
extern int yywrap();
extern int yylex();

extern bool have_yy_error;

#define YYDEBUG 0

#define YYSTYPE string
#define YY_SKIP_YYWRAP

extern std::string module_string;

extern int yylineno;
extern char *yytext;
extern FILE *yyin;

extern YYSTYPE yylval;

#if defined(BISON)
#include "c_alike.tab.hpp"
#else
#include "c_alike.tab.cpp.h"
#endif
