%{
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

%}


%token L_ID L_FLOATCONST L_INTCONST
%token L_BOOLCONST_TRUE L_BOOLCONST_FALSE
%token L_MODULE L_SCRIPT L_IMPORT L_RETURN L_GLOBALS
%token L_IF L_THEN L_ELSE
%token L_WHILE
%token L_EQUAL L_NOT_EQUAL L_GREATER_OR_EQUAL L_LESSER_OR_EQUAL
%token L_BOOL_AND L_BOOL_OR
%token L_INT L_FLOAT L_BOOL L_OBJECT L_VOID
%token L_BEGINSCRIPT L_ENDSCRIPT L_BEGINBLOCK L_ENDBLOCK

%left '-' '+'
%left '*' '/'

%%

module:		L_MODULE  L_ID '{' module_body '}'	{
	string module="<module name="+$2+" >\n"+$4+"\n</module>\n";
	//	printf("%s\n",module.c_str());
	module_string=module;
};
module_body:	/* empty */   { $$=""; }
		| module_body module_statement ';' {
	$$=$1+"\n"+$2;
}		;
module_statement:	script  { $$=$1; } | defvar {$$=$1}; | import { $$=$1 }; | globals { $$=$1; };
globals:	L_GLOBALS '{' globals_body '}' {
	$$="<globals>\n"+$3+"\n</globals>\n";
};
globals_body:	/* empty */   { $$=""; }
		| globals_body global_statement ';' {
	$$=$1+"\n"+$2;
};
global_statement:	defvar { $$=$1; };
import:		L_IMPORT L_ID 	{
	$$="<import name="+$2+"/>";
 };
vartype:	inttype | floattype | booltype | objecttype ;

inttype:        L_INT  		{ $$="int" };
floattype:      L_FLOAT 	  { $$="float"};
booltype:       L_BOOL		{  $$="bool"};
objecttype:     L_OBJECT 	 { $$="object"};
voidtype:	L_VOID		{ $$="void";};
defvar:		vartype L_ID '=' constant {
	$$="<defvar name="+$2+" type="+$1+" value="+$4+" />\n";
}
		| vartype L_ID	{
	$$="<defvar name="+$2+" type="+$1+"/>\n";
};
script:		script_header '{' script_body '}'	{
	$$=$1+"\n"+$3+"\n</script>\n";
};
script_header:	vartype L_ID '(' arguments ')'	{
	$$="<script name="+$2+" return="+$1+" >\n"+"<arguments>\n"+$4+"\n</arguments>\n";
};
arguments:	/* empty */	{ $$="\n"; }
	| arguments  argument ';' {
	$$=$1+"\n"+$2;
};
argument:	defvar 	{
	$$=$1;
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
			| setvar { $$=$1; }
			| call_void { $$=$1; }
			| while_statement { $$=$1; }
			| return_statement { $$=$1; };
return_statement: 	L_RETURN	{
	$$="<return/>\n";
}
			| L_RETURN expr {
	$$="<return>\n"+$2+"\n</return>\n";
}

while_statement: L_WHILE expr block_statement	{
	$$="<while>\n"+$2+"\n"+$3+"\n</while>\n";
};
call_void:	L_ID '.' L_ID '(' call_arglist ')'	{
	if($1[0]=='_'){
		$$="<call module="+$1+" name="+$3+" >\n"+$5+"\n</call>\n";
	}
	else{	
		$$="<exec module="+$1+" name="+$3+" >\n"+$5+"\n</exec>\n";
	}
}
		| L_ID '(' call_arglist ')'	{
	$$="<exec name="+$1+" >\n"+$3+"\n</exec>\n";
};
call_arglist:	/* empty */	{ $$="\n"; }
	| call_arglist  expr ',' {
	$$=$1+"\n"+$2;
};
setvar:			L_ID '=' expr   {
	$$="<setvar name="+$1+" >\n"+$3+"\n</setvar>\n";
}
if_statement:		L_IF '(' expr ')' block_statement L_ELSE block_statement	{
	$$="<if>\n"+$3+"\n"+$5+"\n"+$7+"\n</if>\n";
};
block_statement:	'{' script_body '}'	{
	$$="<block>\n"+$2+"\n</block>\n";
};
constant:	number { $$=$1; } | boolconst	{ $$=$1; };
boolconst:	L_BOOLCONST_TRUE  { $$="true"; }
		| L_BOOLCONST_FALSE { $$="false"; };
number:			L_FLOATCONST {
	 $$="<const type=float value="+$1+" />\n";
 }
			| L_INTCONST	{
	 $$="<const type=int value="+$1+" />\n";
}
expr:			constant	{ $$=$1; };
			| L_ID {
	$$="<var name="+$1+" />\n";
}
			| L_ID '.' L_ID {
	$$="<var module="+$1+" name="+$2+" />\n";
}
			| call_void { $$=$1; }
			| expr L_EQUAL expr
{ $$="<test test=eq >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr L_NOT_EQUAL expr
{ $$="<test test=ne >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr L_LESSER_OR_EQUAL expr
{ $$="<test test=le >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr L_GREATER_OR_EQUAL expr
{ $$="<test test=ge >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr '<' expr
{ $$="<test test=lt >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr '>' expr
{ $$="<test test=gt >\n"+$1+"\n"+$3+"\n</test>\n"; }

			| expr '*' expr
{ $$="<fmath math=* >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr '/' expr
{ $$="<fmath math=/ >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr '-' expr
{ $$="<fmath math=- >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr '+' expr
{ $$="<fmath math=+ >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr L_BOOL_AND expr
{ $$="<and>\n"+$1+"\n"+$3+"\n</and>\n"; }
			| expr L_BOOL_OR expr
{ $$="<or>\n"+$1+"\n"+$3+"\n</or>\n"; }
			| '!' expr
{ $$="<not>\n"+$2+"\n</not>\n";		}
			| '(' expr ')'
{ $$=$2; };

%%

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
