%{

#if 0
#include <vector>
#include <string>

	vector<string> pstack;
#endif

#include <malloc.h>

extern int yyerror(char *);
extern int yywrap();
extern int yylex();

#define YYDEBUG 0

  typedef char* str;
#define YYSTYPE str

%}


%token L_ID L_FLOATCONST L_INTCONST
%token L_MODULE L_SCRIPT L_IMPORT
%token L_IF L_THEN L_ELSE
%token L_EQUAL L_NOT_EQUAL
%token L_INT L_FLOAT L_BOOL L_OBJECT
%token L_BEGINSCRIPT L_ENDSCRIPT L_BEGINBLOCK L_ENDBLOCK

%left '-' '+'
%left '*' '/'

%%

module:		L_MODULE  module_body 		{ printf("<module>\n"); };
module_body:	/* empty */
		| module_body module_statement ';'		;
module_statement:	script | defvar | import	;
import:		L_IMPORT L_ID 	{ printf("<import name=%s>\n",$2); }			;
vartype:	L_INT | L_FLOAT | L_BOOL		;
defvar:		vartype L_ID	{ printf("<defvar name=%s>\n",$2); }				;
script:		L_SCRIPT '{' script_body '}'		;
script_body:	/* empty */
		| script_body script_statement ';'		;
script_statement:	if_statement
			| block_statement
			| defvar
			| setvar		;
setvar:			L_ID '=' expr		;
if_statement:		L_IF '(' expr ')' block_statement L_ELSE block_statement	;
block_statement:	'{' script_body '}'			;
number:			L_FLOATCONST | L_INTCONST		;
expr:			number
			| L_ID
			| expr L_EQUAL expr
			| expr L_NOT_EQUAL expr
			| expr '*' expr
			| expr '/' expr
			| expr '-' expr
			| expr '+' expr
			| '(' expr ')'				;

%%

/* nothing */


#include <stdio.h>


/* extern int yydebug; */

void main()
{
  extern FILE *yyin;
  yyin=fopen("test.c","r");
/*  yydebug=1; */
  yyparse();
}

extern	int yylineno;
extern char *yytext;


int yyerror(char *s){
  printf("(yy)error: %s line %d text -%s-\n",s,yylineno,yytext);
}

int yywrap(){
	return 1;
}
