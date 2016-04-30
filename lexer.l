%{
typedef void * ExprAST;
typedef void * FunctionAST;
typedef void * PrototypeAST;
typedef void * StringList;
typedef void * ExprList;

#include "parser.hpp"
%}

%%

[ \n\t] {}

#[^\n]* {}

[,()] { return *yytext; }

[-+*/] { yylval.c = strdup(yytext); return OPER; }

def { return DEF; }
extern { return EXTERN; }

[0-9]([.][0-9]+)? { yylval.d = strtod(yytext, 0); return NUMBER; }

[A-Za-z][A-Za-z0-9]* { yylval.c = strdup(yytext); return IDENTIFIER; }

%%

