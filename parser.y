%{
#include "main.hpp"

extern "C" int yylex(void);
extern "C" void yyerror(const char *);
%}

%union {
    double d;
    const char * c;

    ExprAST * ex;
    FunctionAST * fn;
    PrototypeAST * pt;

    StringList * ptl;
    ExprList * exl;
}

%token DEF
%token EXTERN
%token IDENTIFIER
%token NUMBER
%token OPER

%type <fn> definition
%type <ex> expression
%type <exl> exprParameters
%type <pt> extern
%type <ex> identifier
%type <ex> number
%type <ex> primary
%type <pt> prototype
%type <ptl> protoParameters
%type <fn> topLevelExpr

%type <c> IDENTIFIER
%type <c> OPER
%type <d> NUMBER

%left '+' '-'
%left '*' '/'

%%

repl: topLevel
    | topLevel repl
    ;

topLevel: definition { processDefinition($1); }
        | extern { processExtern($1); }
        | topLevelExpr { processTopLevelExpr($1); }
        ;

topLevelExpr: expression { $$ = new FunctionAST(new PrototypeAST("", std::vector<std::string>()), $1); }
            ;

definition: DEF prototype expression { $$ = new FunctionAST($2, $3); }
          ;

extern: EXTERN prototype { $$ = $2; }
      ;

prototype: IDENTIFIER '(' protoParameters ')' { $$ = new PrototypeAST($1, *$3); }
         ;

protoParameters: IDENTIFIER { $$ = new std::vector<std::string>(); $$->push_back($1); }
               | IDENTIFIER ',' protoParameters { $$ = $3; $3->insert($$->begin(), $1); }
               ;

expression: primary { $$ = $1; }
          | expression OPER expression { $$ = new BinaryExprAST(*$2, $1, $3); }
          ;

primary: identifier { $$ = $1; }
       | number { $$ = $1; }
       | '(' expression ')' { $$ = $2; }
       ;

identifier: IDENTIFIER { $$ = new VariableExprAST($1); }
          | IDENTIFIER '(' exprParameters ')' { $$ = new CallExprAST($1, *$3); }
          ; 

exprParameters: expression { $$ = new std::vector<ExprAST *>(); $$->push_back($1); }
              | expression ',' exprParameters { $$ = $3; $3->insert($3->begin(), $1); }
              ;

number: NUMBER { $$ = new NumberExprAST($1); }
      ;

