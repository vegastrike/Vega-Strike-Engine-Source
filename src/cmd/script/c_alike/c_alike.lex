%{

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


#include <stdio.h>
#include <malloc.h>

#include <vector>
#include <string>

  typedef char* str;
#define YYSTYPE string
#define YY_SKIP_YYWRAP

#include "c_alike.tab.cpp.h"
#include "stdio.h"
#include "string.h"

    extern int yywrap();

  //  extern char *yylval;

%}

%option yylineno

L_DIGIT		[0-9]
L_INTCONST	{L_DIGIT}+
L_FLOATCONST 	{L_DIGIT}+"."{L_DIGIT}+
L_ID   		[a-z][a-z0-9]*


%%
module		{ return(L_MODULE);		}
script		{  return(L_SCRIPT);	}
import		{  return(L_IMPORT); 		}
return		{ return(L_RETURN);	}
globals		{ return(L_GLOBALS);	}

if		{ return(L_IF);			}
then		{ return(L_THEN);			}
else		{ return(L_ELSE);			}

"=="		{ return(L_EQUAL);			}
"!="		{ return(L_NOT_EQUAL);			}
">="		{ return(L_GREATER_OR_EQUAL);			}
"<="		{ return(L_LESSER_OR_EQUAL);			}

"&&"		{ return(L_BOOL_AND);			}
"||"		{ return(L_BOOL_OR);			}

true		{ return(L_BOOLCONST_TRUE); }
false		{ return(L_BOOLCONST_FALSE);	}

while		{ return(L_WHILE);		}

int		{ 	 return(L_INT);		}
float		{ return(L_FLOAT);			}
bool		{ return(L_BOOL);			}
object		{ return(L_OBJECT);			}
void		{ return(L_VOID);		}
"->"		{ return(L_METHODCALL);		}
":="		{ return(L_INITVALUE);		}

[ \t\n]+	{ /* return(yytext[0]); */			}

[a-zA-Z_][a-zA-Z_0-9]*		{  yylval=strdup(yytext); return(L_ID);		}
[0-9]+		{ yylval=strdup(yytext); return(L_INTCONST);	}
[0-9]+\.[0-9]+	{  yylval=strdup(yytext); return(L_FLOATCONST);		}

[\{\}();=]	{ return(yytext[0]); }
\"[^\"]*\" 		{  yylval=strdup(yytext); return(L_STRINGCONST); };
"/""*"(.|\n)*"*""/" { /* nothing */ }
"//".*\n	{ /* nothing */ };

.		{  return(yytext[0]); }
%%

/* nothing */
