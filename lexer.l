%{
#include "parser.h"
%}

%%

[ \n\t] {}

#[^\n]* {}

def { return DEF; }
extern { return EXTERN; }

[0-9]([.][0-9]+)? { yylval.d = strtod(yytext, 0); return NUMBER; }

[A-Za-z][A-Za-z0-9]* { yylval.c = strdup(yytext); return IDENTIFIER; }

%%

