#include <stdio.h>
#include <string.h>
#include "front-end/scanner.h"
#include "front-end/ast.h"

void scan(char* code, Token* buf_toks);
void parse(Token* tokens, Node** program);
void print_node(FILE* f, Node* n);
void free_node(Node* n);

int main(int argc, char** argv) {
    if (!(argc >= 3)) {
        printf("Error: Expected a file to compile and a file to write output to\nAborting execution\n");
        return 0;
    }
    char code[1024];
    char c;

    memset(code, 0, sizeof(code));
    char file[sizeof(argv[1]) + 4] = "K:/";
    int size = sizeof(argv[1]);
    strncpy(file, argv[1], size);
    printf("%s\n", file);
    FILE* input_f = fopen(file, "r");
    fread(code, 1, sizeof(code), input_f);
    printf("code is %s in compiler.c\n", code);
    Token tokens[sizeof(code)];
    memset(tokens, 0, sizeof(tokens));
    Node* program[1024];
    memset(program, 0, sizeof(program));
    scan(code, tokens);
    parse(tokens, program);
    FILE* output_f = fopen(argv[2], "w");
    for (int i = 0; i < sizeof(program) / sizeof(Node*); i++) {
        if (NULL != program[i]) {
            print_node(output_f, program[i]);
            fprintf(output_f, "\n");
        }
    }
    fclose(input_f);
    fclose(output_f);
    for (int i = 0; i < sizeof(program) / sizeof(Node*); i++) {
        free_node(program[i]);
    }
    return 0;
}
