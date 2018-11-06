%{
#include <iostream>
#include "datadecl.tab.h"
#include <stdlib.h>
extern int yylex();
extern unsigned lineNum;
%}
%option noyywrap
%%

\/\/.*         {}

[ \t]*          {}                        

\n              {lineNum++;}
       
=               { return EQUALS;}
value           { return VALUE;}
array           { return ARRAY;}
struct          { return STRUCT;}
structarray     { return STRUCTARRAY; }
structinstance  { return STRUCTINSTANCE; }
structarrayinstance {  return STRUCTARRAYINSTANCE; }
low             { return LOW;}
high            {  return HIGH;}
bins            {  return BINS; }
units           {  return UNITS; }
\[              {  return LBRACK; }
\]              {  return RBRACK; }
\{              {  return LCURLY; }
\}              {  return RCURLY; }
-?[0-9]+        {
                    yylval.number = strtod(yytext, NULL);
                    return NUMBER; }
-?[0-9]+.[0-9]+ {
                    yylval.number = strtod(yytext, NULL);
                    return NUMBER; 
                    }

[A-Z,a-z][A-Z,a-z,0-9,$,_]* {
                                yylval.str = strdup(yytext);
                                return NAME;}

.               {}

%%


