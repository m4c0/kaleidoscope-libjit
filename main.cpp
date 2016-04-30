#include "main.hpp"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>

extern int yyparse();
extern FILE * yyin;

extern "C" int yywrap() {
    return 1;
}
extern "C" void yyerror(const char * str) {
    std::cerr << "Error parsing: " << str << std::endl;
}

void processDefinition(FunctionAST *) {
    std::cout << "Definition" << std::endl;
}
void processExtern(PrototypeAST *) {
    std::cout << "External" << std::endl;
}
void processTopLevelExpr(FunctionAST *) {
    std::cout << "Top Level" << std::endl;
}

int main() {
    yyin = stdin;
    yyparse();
    return 0;
}

