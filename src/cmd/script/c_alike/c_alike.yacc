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


#include "c_alike.h"
std::string module_string;

bool have_yy_error;

//string module_string;

string lineno(){
char buffer[100];

	sprintf(buffer," line=\"%d\" ",yylineno+1);

	return buffer;
}

%}


%token L_ID L_FLOATCONST L_INTCONST L_STRINGCONST
%token L_BOOLCONST_TRUE L_BOOLCONST_FALSE
%token L_MODULE L_SCRIPT L_IMPORT L_RETURN L_GLOBALS L_CLASS
%token L_IF L_THEN L_ELSE
%token L_WHILE
%token L_EQUAL L_NOT_EQUAL L_GREATER_OR_EQUAL L_LESSER_OR_EQUAL
%token L_BOOL_AND L_BOOL_OR
%token L_INT L_FLOAT L_BOOL L_OBJECT L_VOID 
%token L_METHODCALL L_INITVALUE

%left '-' '+'
%left '*' '/'
%right '!'

%%

module:		L_MODULE  L_ID '{' module_body '}'	{
	string module="<module  "+lineno()+" name=" + q($2) + " >\n"+$4+"\n</module>\n";
	module_string=module;
};
module_body:	/* empty */   { $$=""; }
		| module_body module_statement ';' {
	$$=$1+"\n"+$2;
}		;
module_statement:	 defvar { $$=$1; } | script  { $$=$1; } | import { $$=$1; } | globals { $$=$1; };

globals:	L_GLOBALS '{' globals_body '}' {
	$$="<globals "+lineno()+" >\n"+$3+"\n</globals>\n";
};
globals_body:	/* empty */   { $$=""; }
		| globals_body global_statement ';' {
	$$=$1+"\n"+$2;
};
global_statement:	defvar { $$=$1; };
import:		L_IMPORT L_ID 	{
	$$="<import  "+lineno()+" name="+q($2)+"/>";
 };
vartype:	inttype {$$=$1;} | floattype {$$=$1;}
		| booltype {$$=$1;} | objecttype {$$=$1;}
		| voidtype { $$=$1;};

inttype:        L_INT  		{ $$="int"; };
floattype:      L_FLOAT 	  { $$="float";};
booltype:       L_BOOL		{  $$="bool";};
objecttype:     L_OBJECT 	 { $$="object";};
voidtype:	L_VOID		{ $$="void";};
argvar:		vartype L_ID {
	$$="<defvar name="+q($2)+" type="+q($1)+"/>\n";
//	printf("DEVFAR %s\n",$2.c_str());
};

classvar:	L_CLASS {
	 $$="true";
}
		| /* empty */	 {
	$$="false";
};

defvar_begin:	vartype  {
	$$=" classvar=\"false\" type="+q($1);
} 	

		| L_CLASS vartype  {
	$$=" classvar=\"true\" type="+q($2);
};

defvar:		defvar_begin L_ID '=' expr {
	$$="<defvar  "+lineno()+" name="+q($2)+" "+$1+" />\n"+"<setvar name="+q($2)+" >\n"+$4+"\n</setvar>\n";
}
		| defvar_begin L_ID L_INITVALUE init_val {
	$$="<defvar  "+lineno()+" name="+q($2)+" "+$1+" initvalue="+q($4)+" />\n";
}
		| defvar_begin L_ID ',' nonnull_idlist {
	string allvars="";

	string list=$2+" "+$4+" ";
	int apos=0;
	int npos=0;
	//printf("names=%s\n",list.c_str());
	do{
		 npos=list.find(" ",apos);
		string news=list.substr(apos,npos-apos);
		//printf("NEW: %s apos=%d npos=%d\n",news.c_str(),apos,npos);
		//allvars=allvars+"<defvar name="+q(news)+" type="+q($1)+" />\n";
		allvars=allvars+"<defvar name="+q(news)+" "+$1+" />\n";
		apos=npos+1;
	}while(apos<list.size()-1);
	
	$$=allvars;
}
		| defvar_begin L_ID {
	$$="<defvar  "+lineno()+" name="+q($2)+" "+$1+" />\n";
};


defvar_old:		classvar vartype L_ID L_INITVALUE init_val {
	$$="<defvar  "+lineno()+" name="+q($3)+" type="+q($2)+" initvalue="+q($5)+" classvar="+q($1)+" />\n";
}
		| vartype L_ID '=' expr {
	$$="<defvar  "+lineno()+" name="+q($2)+" type="+q($1)+" />\n"+"<setvar name="+q($2)+" >\n"+$4+"\n</setvar>";
}
		|  vartype L_ID	{
	$$="<defvar  "+lineno()+" name="+q($2)+" type="+q($1)+"/>\n";
	printf("DEVFAR %s\n",$2.c_str());
}
		| vartype L_ID ',' nonnull_idlist {

	string allvars="";

	string list=$2+" "+$4+" ";
	int apos=0;
	int npos=0;
	//printf("names=%s\n",list.c_str());
	do{
		 npos=list.find(" ",apos);
		string news=list.substr(apos,npos-apos);
		//printf("NEW: %s apos=%d npos=%d\n",news.c_str(),apos,npos);
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
script_header:	defvar_begin L_ID '(' arguments ')'	{
//	$$="<script  "+lineno()+" name="+q($2)+" return="+q($1)+" >\n"+"<arguments>\n"+$4+"\n</arguments>\n";
	$$="<script  "+lineno()+" name="+q($2)+" "+$1+" >\n"+"<arguments>\n"+$4+"\n</arguments>\n";
};
var_or_voidtype:	vartype { printf("var_or_voidtype\n"); $$=$1;}
	| voidtype {$$=$1;};

arguments:	/* empty */	{ $$="\n"; }
	| nonnull_arguments  { $$=$1;};

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
	$$="<return "+lineno()+" />\n";
}
			| L_RETURN expr {
	$$="<return "+lineno()+" >\n"+$2+"\n</return>\n";
};

while_statement: L_WHILE expr block_statement	{
	$$="<while "+lineno()+" >\n"+$2+"\n"+$3+"\n</while>\n";
};
call_void:	L_ID '.' L_ID '(' attributes call_arglist ')'	{
	if($1[0]=='_'){
		$$="<call  "+lineno()+" module="+q($1)+" name="+q($3)+" "+$5+" >\n"+$6+"\n</call>\n";
	}
	else{	
		$$="<exec  "+lineno()+" module="+q($1)+" name="+q($3)+" "+$5+" >\n"+$6+"\n</exec>\n";
	}
}

		| L_ID L_METHODCALL L_ID '(' attributes call_arglist ')'	{
		$$="<call  "+lineno()+" object="+q($1)+" name="+q($3)+" "+$5+" >\n"+$6+"\n</call>\n";
}
		| L_ID '(' call_arglist ')'	{
	$$="<exec  "+lineno()+" name="+q($1)+" >\n"+$3+"\n</exec>\n";
};
call_arglist:	/* empty */	{ $$="\n"; }
	| nonnull_arglist  { $$=$1;};

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
	$$="<setvar name="+q($1)+lineno()+" >\n"+$3+"\n</setvar>\n";
};
if_statement:	L_IF '(' expr ')' block_statement L_ELSE block_statement	{
	$$="<if "+lineno()+" >\n"+$3+"\n"+$5+"\n"+$7+"\n</if>\n";
}
		| L_IF '(' expr ')' block_statement L_ELSE if_statement	{
	$$="<if "+lineno()+" >\n"+$3+"\n"+$5+"\n"+$7+"\n</if>\n";
}
		| L_IF '(' expr ')' block_statement	{
	$$="<if "+lineno()+" >\n"+$3+"\n"+$5+"\n"+"<block></block>"+"\n</if>\n";
};
block_statement:	'{' script_body '}'	{
	$$="<block "+lineno()+" >\n"+$2+"\n</block>\n";
};
constant:	number { $$=$1; } | boolconst	{ $$=$1; }
		| stringconst {$$=$1;};
stringconst:	string_constant	{
	$$="<const "+lineno()+"  type=\"object\" object=\"string\" value="+$1+" />\n";
};
string_constant:	L_STRINGCONST {$$=$1;};

init_val:	boolvalue {$$=$1;}
		| L_FLOATCONST	{$$=$1;}
		| L_INTCONST	{$$=$1;};

boolvalue:	L_BOOLCONST_TRUE  {
	 $$="true";
 }
		| L_BOOLCONST_FALSE {
	 $$="false";
};	
boolconst:	boolvalue  {
	 $$="<const  "+lineno()+" type=\"bool\" value="+q($1)+" />\n";
 };

number:			L_FLOATCONST {
	 $$="<const  "+lineno()+" type=\"float\" value="+q($1)+" />\n";
 }
			| L_INTCONST	{
	 $$="<const  "+lineno()+" type=\"int\" value="+q($1)+" />\n";
};
expr:			constant	{ $$=$1; }
			| L_ID {
	$$="<var  "+lineno()+" name="+q($1)+" />\n";
}
			| L_ID '.' L_ID {
	$$="<var  "+lineno()+" module="+q($1)+" name="+q($2)+" />\n";
}
			| call_void { $$=$1; }
			| expr L_EQUAL expr
{ $$="<test  "+lineno()+" test=\"eq\" >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr L_NOT_EQUAL expr
{ $$="<test  "+lineno()+" test=\"ne\" >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr L_LESSER_OR_EQUAL expr
{ $$="<test  "+lineno()+" test=\"le\" >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr L_GREATER_OR_EQUAL expr
{ $$="<test  "+lineno()+" test=\"ge\" >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr '<' expr
{ $$="<test  "+lineno()+" test=\"lt\" >\n"+$1+"\n"+$3+"\n</test>\n"; }
			| expr '>' expr
{ $$="<test  "+lineno()+" test=\"gt\" >\n"+$1+"\n"+$3+"\n</test>\n"; }

			| expr '*' expr
{ $$="<fmath  "+lineno()+" math=\"*\" >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr '/' expr
{ $$="<fmath  "+lineno()+" math=\"/\" >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr '-' expr
{ $$="<fmath  "+lineno()+" math=\"-\" >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr '+' expr
{ $$="<fmath  "+lineno()+" math=\"+\" >\n"+$1+"\n"+$3+"\n</fmath>\n"; }
			| expr L_BOOL_AND expr
{ $$="<and "+lineno()+" >\n"+$1+"\n"+$3+"\n</and>\n"; }
			| expr L_BOOL_OR expr
{ $$="<or "+lineno()+" >\n"+$1+"\n"+$3+"\n</or>\n"; }
			| '!' expr
{ $$="<not "+lineno()+" >\n"+$2+"\n</not>\n";		}
			| '(' expr ')'
{ $$=$2; };

%%

/* nothing */


/* extern int yydebug; */

string parseCalike(char const *filename)
{
  string empty_string;

  have_yy_error=false;

  yylineno=0;

  yyin=fopen(filename,"r");

  if(yyin==NULL){
    // file not found
    return empty_string;
  }

/*  yydebug=1; */
  yyparse();
  
  return module_string;
}


int yyerror(char *s){
  printf("(yy)error: %s line %d text -%s-\n",s,yylineno+1,yytext);
  have_yy_error=true;

  return 1;
}

int yywrap(){
	return 1;
}



