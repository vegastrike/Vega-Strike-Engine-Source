%{
#include <stdio.h>
#include <malloc.h>

#include <vector>
#include <string>


extern int yyerror(char *);
extern int yywrap();
extern int yylex();

#define YYDEBUG 0

  typedef char* str;
#define YYSTYPE string

	vector<string> pstack;

string Pop(){
	string res=pstack.back();
	pstack.pop_back();
	return res;
}

string printStack(){
	int len=pstack.size();
	string res;

	for(int i=0;i<len;i++){
		printf("stack #%2d  = -%s-\n",i,pstack[i].c_str());
		res=res+pstack[i];
	}
	return res;
}

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

module:		L_MODULE  module_body 	{
	printf("module is:\n");
	printf("%s\n",$2.c_str());
};
module_body:	/* empty */   { $$=""; }
		| module_body module_statement ';' {
	$$=$1+"\n"+$2;
}		;
module_statement:	script  { $$=$1; } | defvar {$$=$1}; | import { $$=$1 };
import:		L_IMPORT L_ID 	{
	$$="<import name="+$2+"/>";
 };
vartype:	inttype | floattype | booltype | objecttype ;

inttype:        L_INT  		{ $$="int" };
floattype:      L_FLOAT 	  { $$="float"};
booltype:       L_BOOL		{  $$="bool"};
objecttype:     L_OBJECT 	 { $$="object"};
defvar:		vartype L_ID	{
	$$="<defvar name="+$2+" type="+$1+"/>\n";
};
script:		L_SCRIPT '{' script_body '}'	{
	$$="<script>\n"+$3+"\n</script>\n";
};
script_body:	/* empty */
		{ $$=""; }
		| script_body script_statement ';'  {
//	$$="\nscript_body\n";
	$$=$1+"\n"+$2;
};
script_statement:	if_statement { $$=$1; }
			| block_statement { $$=$1; }
			| defvar  { $$=$1; }
			| setvar { $$=$1; };

setvar:			L_ID '=' expr   {
	$$="<setvar name="+$1+" >\n"+$3+"\n</setvar>\n";
}
if_statement:		L_IF '(' expr ')' block_statement L_ELSE block_statement	{
	$$="<if>\n"+$3+"\n"+$5+"\n"+$7+"\n</if>\n";
};
block_statement:	'{' script_body '}'	{
	$$="<block>\n"+$2+"\n</block>\n";
};
number:			L_FLOATCONST {
	 $$="<const type=float value="+$1+" />\n";
 }
			| L_INTCONST	{
	 $$="<const type=int value="+$1+" />\n";
}
expr:			number	{ $$=$1; };
			| L_ID {
	$$="<var name="+$1+" />\n";
}
			| expr L_EQUAL expr
{ $$="<test test=eq >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr L_NOT_EQUAL expr
{ $$="<test test=ne >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr '*' expr
{ $$="<fmath math=* >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr '/' expr
{ $$="<fmath math=/ >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr '-' expr
{ $$="<fmath math=- >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr '+' expr
{ $$="<fmath math=+ >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| '(' expr ')'
{ $$=$2; };

%%

/* nothing */


/* extern int yydebug; */

void main()
{
  extern FILE *yyin;
  yyin=fopen("test.c","r");
/*  yydebug=1; */

	pstack.push_back("");

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
