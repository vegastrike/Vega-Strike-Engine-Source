%{
#include <stdio.h>
#include <malloc.h>

#include <vector>
#include <string>

#define YYERROR_VERBOSE

#define q(x)	("\""+x+"\"")

extern int yyerror(char *);
extern int yywrap();
extern int yylex();

#define YYDEBUG 0

  typedef char* str;
#define YYSTYPE string

string module_string;

%}


%token L_ID L_FLOATCONST L_INTCONST L_STRINGCONST
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
	string module="<module name=" + q($2) + " >\n"+$4+"\n</module>\n";
	module_string=module;
};
module_body:	/* empty */   { $$=""; }
		| module_body module_statement ';' {
	$$=$1+"\n"+$2;
}		;
module_statement:	script  { $$=$1; } | defvar {$$=$1} | import { $$=$1 } | globals { $$=$1; };

globals:	L_GLOBALS '{' globals_body '}' {
	$$="<globals>\n"+$3+"\n</globals>\n";
};
globals_body:	/* empty */   { $$=""; }
		| globals_body global_statement ';' {
	$$=$1+"\n"+$2;
};
global_statement:	defvar { $$=$1; };
import:		L_IMPORT L_ID 	{
	$$="<import name="+q($2)+"/>";
 };
vartype:	inttype {$$=$1;} | floattype {$$=$1;}
		| booltype {$$=$1;} | objecttype {$$=$1}
		| voidtype { $$=$1;};

inttype:        L_INT  		{ $$="int" };
floattype:      L_FLOAT 	  { $$="float"};
booltype:       L_BOOL		{  $$="bool"};
objecttype:     L_OBJECT 	 { $$="object"};
voidtype:	L_VOID		{ $$="void"};
argvar:		vartype L_ID {
	$$="<defvar name="+q($2)+" type="+q($1)+"/>\n";
//	printf("DEVFAR %s\n",$2.c_str());
}

defvar:		vartype L_ID '=' constant {
	$$="<defvar name="+q($2)+" type="+q($1)+" value="+q($4)+" />\n";
}
		| vartype L_ID	{
	$$="<defvar name="+q($2)+" type="+q($1)+"/>\n";
//	printf("DEVFAR %s\n",$2.c_str());
}
		| vartype L_ID ',' nonnull_idlist {

	string allvars="";

	string list=$2+" "+$4+" ";
	int apos=0;
	int npos=0;
	printf("names=%s\n",list.c_str());
	do{
		 npos=list.find(" ",apos);
		string news=list.substr(apos,npos-apos);
		printf("NEW: %s apos=%d npos=%d\n",news.c_str(),apos,npos);
		allvars=allvars+"<defvar name="+q(news)+" type="+q($1)+" />\n";
		apos=npos+1;
	}while(apos<list.size()-1);
	
	$$=allvars;
};
nonnull_idlist:		L_ID { $$=$1;}
	| L_ID ',' nonnull_idlist {
	$$=$1+" "+$3;
};

script:		script_header '{' script_body '}'	{
	$$=$1+"\n"+$3+"\n</script>\n";
};
script_header:	vartype L_ID '(' arguments ')'	{
	$$="<script name="+q($2)+" return="+q($1)+" >\n"+"<arguments>\n"+$4+"\n</arguments>\n";
};
var_or_voidtype:	vartype { printf("var_or_voidtype\n"); $$=$1;}
	| voidtype {$$=$1;};

arguments:	/* empty */	{ $$="\n"; }
	| nonnull_arguments  { $$=$1;}

nonnull_arguments:
	argument { $$=$1;}
	| nonnull_arguments ',' argument {
	$$=$1+"\n"+$3;
};

argument:	argvar 	{
	$$=$1;
};

script_body:	/* empty */
		{ $$=""; }
		| script_body script_statement   {
	$$=$1+"\n"+$2;
};
script_statement:	if_statement { $$=$1; }
			| block_statement { $$=$1; }
			| defvar  ';' { $$=$1; }
			| setvar ';' { $$=$1; }
			| call_void ';' { $$=$1; }
			| while_statement { $$=$1; }
			| return_statement ';' { $$=$1; }
			| ';' { $$=" ";};

return_statement: 	L_RETURN	{
	$$="<return/>\n";
}
			| L_RETURN expr {
	$$="<return>\n"+$2+"\n</return>\n";
}

while_statement: L_WHILE expr block_statement	{
	$$="<while>\n"+$2+"\n"+$3+"\n</while>\n";
};
call_void:	L_ID '.' L_ID '(' attributes call_arglist ')'	{
	if($1[0]=='_'){
		$$="<call module="+q($1)+" name="+q($3)+" "+$5+" >\n"+$6+"\n</call>\n";
	}
	else{	
		$$="<exec module="+q($1)+" name="+q($3)+" "+$5+" >\n"+$6+"\n</exec>\n";
	}
}
		| L_ID '(' call_arglist ')'	{
	$$="<exec name="+q($1)+" >\n"+$3+"\n</exec>\n";
};
call_arglist:	/* empty */	{ $$="\n"; }
	| nonnull_arglist  { $$=$1;}

nonnull_arglist:	expr {$$=$1;}
	| nonnull_arglist ',' expr {
	$$=$1+"\n"+$3;
};

attributes:	/* empty */	{ $$=" "; }
	| attributes  attribute ';' {
	$$=$1+" "+$2;
};
attribute:	':' L_ID '=' string_constant 	 {
	$$=$2+"="+$4+" ";
};


setvar:			L_ID '=' expr   {
	$$="<setvar name="+q($1)+" >\n"+$3+"\n</setvar>\n";
}
if_statement:	L_IF '(' expr ')' block_statement L_ELSE block_statement	{
	$$="<if>\n"+$3+"\n"+$5+"\n"+$7+"\n</if>\n";
}
		| L_IF '(' expr ')' block_statement L_ELSE if_statement	{
	$$="<if>\n"+$3+"\n"+$5+"\n"+$7+"\n</if>\n";
}
		| L_IF '(' expr ')' block_statement	{
	$$="<if>\n"+$3+"\n"+$5+"\n"+"<block></block>"+"\n</if>\n";
};
block_statement:	'{' script_body '}'	{
	$$="<block>\n"+$2+"\n</block>\n";
};
constant:	number { $$=$1; } | boolconst	{ $$=$1; };
string_constant:	L_STRINGCONST {$$=$1;};

boolconst:	L_BOOLCONST_TRUE  { $$="true"; }
		| L_BOOLCONST_FALSE { $$="false"; };
number:			L_FLOATCONST {
	 $$="<const type=\"float\" value="+q($1)+" />\n";
 }
			| L_INTCONST	{
	 $$="<const type=\"int\" value="+q($1)+" />\n";
}
expr:			constant	{ $$=$1; };
			| L_ID {
	$$="<var name="+q($1)+" />\n";
}
			| L_ID '.' L_ID {
	$$="<var module="+q($1)+" name="+q($2)+" />\n";
}
			| call_void { $$=$1; }
			| expr L_EQUAL expr
{ $$="<test test=\"eq\" >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr L_NOT_EQUAL expr
{ $$="<test test=\"ne\" >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr L_LESSER_OR_EQUAL expr
{ $$="<test test=\"le\" >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr L_GREATER_OR_EQUAL expr
{ $$="<test test=\"ge\" >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr '<' expr
{ $$="<test test=\"lt\" >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr '>' expr
{ $$="<test test=\"gt\" >\n"+$1+"\n"+$3+"\n</test>\n"; }

			| expr '*' expr
{ $$="<fmath math=\"*\" >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr '/' expr
{ $$="<fmath math=\"/\" >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr '-' expr
{ $$="<fmath math=\"-\" >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr '+' expr
{ $$="<fmath math=\"+\" >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
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


#if 0

expr:   nonnull_exprlist
                { $$ = build_compound_expr ($1); }
        ;

exprlist:
          /* empty */
                { $$ = NULL_TREE; }
        | nonnull_exprlist
        ;
 
nonnull_exprlist:
        expr_no_commas
                { $$ = build_tree_list (NULL_TREE, $1); }
        | nonnull_exprlist ',' expr_no_commas
                { chainon ($1, build_tree_list (NULL_TREE, $3)); }
        ;

expr_no_commas:
          cast_expr
        | expr_no_commas '+' expr_no_commas
                { $$ = parser_build_binary_op ($2, $1, $3); }
        | expr_no_commas '-' expr_no_commas
                { $$ = parser_build_binary_op ($2, $1, $3); }
        | expr_no_commas '*' expr_no_commas
                { $$ = parser_build_binary_op ($2, $1, $3); }
        | expr_no_commas '/' expr_no_commas
                { $$ = parser_build_binary_op ($2, $1, $3); }
        | expr_no_commas '%' expr_no_commas
                { $$ = parser_build_binary_op ($2, $1, $3); }
        | expr_no_commas LSHIFT expr_no_commas
                { $$ = parser_build_binary_op ($2, $1, $3); }
        | expr_no_commas RSHIFT expr_no_commas
                { $$ = parser_build_binary_op ($2, $1, $3); }
        | expr_no_commas ARITHCOMPARE expr_no_commas
                { $$ = parser_build_binary_op ($2, $1, $3); }
        | expr_no_commas EQCOMPARE expr_no_commas
                { $$ = parser_build_binary_op ($2, $1, $3); }


#endif
