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
#define YYSTYPE str

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
	 printf("<module>\n");
	string module="<module>\n";

	module=module+Pop();

	module=module+"\n</module>\n\n";

	printf("ENDRESULT: %s\n",module.c_str());
};
module_body:	/* empty */
		| module_body module_statement ';'		;
module_statement:	script  | defvar | import 	{
	string statement=Pop();
	string body=Pop();
	body=body+statement;
	pstack.push_back(body);
};
import:		L_IMPORT L_ID 	{
	 printf("<import name=%s/>\n",$2);
	string imp="<import name="+string($2)+">\n";
	pstack.push_back(imp);
 };
vartype:	inttype | floattype | booltype | objecttype ;

inttype:        L_INT  		{ pstack.push_back("int"); $$=strdup("blah")};
floattype:      L_FLOAT 	  { pstack.push_back("float"); };
booltype:       L_BOOL		{ pstack.push_back("bool"); };
objecttype:     L_OBJECT 	 { pstack.push_back("object"); };
defvar:		vartype L_ID	{
	string type=Pop();
	 printf("<defvar 1=%s name=%s type=%s/>\n",$$,$2,type.c_str());
	string defvar="<defvar name="+string($2)+" type="+type+">\n";
	pstack.push_back(defvar);
};
script:		L_SCRIPT '{' script_body '}'	{

	string script="<script>\n";
#if 0
	script=script+printStack();
	script=script+"\n</script>\n";
#endif
	script=script+Pop();
	pstack.push_back(script);
};
script_body:	/* empty */
		| script_body script_statement ';'  {
	printf("SCRIPT_BODY\n");
	string forget=printStack();

	string last_statement=Pop();

	string body=Pop();
	body=body+last_statement;
	pstack.push_back(body);
};
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
