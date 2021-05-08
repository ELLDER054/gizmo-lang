#include <stdio.h>
#include <string.h>
#include "front-end/scanner.c"
#include "front-end/ast.c"
#include "front-end/parser.c"
#include "front-end/scanner.h"
#include "front-end/ast.h"

void scan(char* code, Token* buf_toks);
void parse(Token* tokens, Node** program);
void print_node(Node* n);
void free_node(Node* n);

int main(int argc, char** argv) {
    if (!(argc >= 3)) {
        printf("Error: Expected a file to compile and a file to write output to\nAborting execution\n");
        return 0;
    }
    char code[1024];
    char c;

    memset(code, 0, sizeof(code));

    FILE* f = fopen(argv[1], "r");
    fread(code, 1, sizeof(code), f);
    Token tokens[sizeof(code)];
    Node* program[1024];
    memset(program, 0, sizeof(program));
    scan(code, tokens);
    parse(tokens, program);
    for (int i = 0; i < sizeof(program) / sizeof(Node*); i++) {
        if (NULL != program[i]) {
            print_node(program[i]);
            printf("\n");
        }
    }
    fclose(f);
    for (int i = 0; i < sizeof(program) / sizeof(Node*); i++) {
        free_node(program[i]);
    }
    return 0;
}
