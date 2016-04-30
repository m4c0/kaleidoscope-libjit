#include <stdio.h>

int yywrap() {
    return 1;
}
void yyerror(const char * str) {
    fprintf(stderr, "Error parsing: %s\n", str);
}

int main() {
    return 0;
}

