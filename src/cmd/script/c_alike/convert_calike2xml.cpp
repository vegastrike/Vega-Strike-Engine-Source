#include <stdio.h>

#include <string>

extern string module_string;

extern void yyparse();

void main(int argc,char **argv)
{
  extern FILE *yyin;
  if(argc<=1){
    yyin=fopen("test.c","r");
  }
  else{
    //    yyin=fopen(argv[1],"r");
    yyin=stdin;
  }

  yyparse();

  cout << module_string;
}

