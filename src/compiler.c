#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "ast.h"

void scan(char code[1024], Token buf_toks[1024]);
void parse(Token tokens[1024], Node** program);
void print_node(Node* n);
void free_node(Node* n);

int main(int argc, char** argv) {
    if (!(argc >= 2)) {
        printf("Error: Expected a file to execute\nAborting execution\n");
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
