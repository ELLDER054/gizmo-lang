#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "ast.h"

void scan(char code[1024], Token buf_toks[1024]);
void parse(Token tokens[1024], Node program[1024]);

int main(void) {
    char code[1024];
    char c;

    memset(code, 0, sizeof(code));

    FILE* f = fopen("test.gizmo", "r");
    fread(code, 1, sizeof(code), f);
    Token tokens[strlen(code)];
    Node program[1028];
    scan(code, tokens);
    parse(tokens, program);
    fclose(f);
    return 0;
}
