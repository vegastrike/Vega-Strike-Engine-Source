/*
 * c_alike.h
 *
 * Copyright (C) 2001-2024 Daniel Horn, pyramid3d, Stephen G. Tuggy, Benjamen R. Meyer,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
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
#elif defined(__APPLE__) && defined(__MACH__)
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
