%{
  typedef char* str;
#define YYSTYPE str
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

if		{ return(L_IF);			}
then		{ return(L_THEN);			}
else		{ return(L_ELSE);			}

"=="		{ return(L_EQUAL);			}
"!="		{ return(L_NOT_EQUAL);			}

int		{ 	 return(L_INT);		}
float		{ return(L_FLOAT);			}
bool		{ return(L_BOOL);			}
object		{ return(L_OBJECT);			}

beginscript	{ return(L_BEGINSCRIPT);		}
endscript	{ return(L_ENDSCRIPT);			}
beginblock	{ return(L_BEGINBLOCK);		}
endblock	{ return(L_ENDBLOCK);		}

[ \t\n]+	{ /* return(yytext[0]); */			}

[a-z][a-z0-9]*		{  yylval=strdup(yytext); return(L_ID);		}
[0-9]+		{ yylval=strdup(yytext); return(L_INTCONST);	}
[0-9]+\.[0-9]+	{  yylval=strdup(yytext); return(L_FLOATCONST);		}

[\{\}();=]	{ return(yytext[0]); }
.		{  return(yytext[0]); }
%%

/* nothing */
