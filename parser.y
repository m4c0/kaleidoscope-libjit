%{
int yylex(void);
void yyerror(const char *);
%}

%union {
    double d;
    const char * c;
}

%token DEF
%token EXTERN
%token IDENTIFIER
%token NUMBER

%%

dunno:

