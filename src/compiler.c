#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "front-end/scanner.h"
#include "front-end/ast.h"
#include "back-end/codegen.h"

typedef struct Symbol Symbol;
void scan(char* code, Token* buf_toks);
void parse(Token* tokens, Node** program, Symbol** symbol_table);
void print_node(FILE* f, Node* n);
void free_node(Node* n);
void generate(Node** ast, int length, char* code);

int main(int argc, char** argv) {
    if (!(argc >= 3)) {
        fprintf(stderr, "Error: Expected a file to compile and a file to write output to\nAborting execution\n");
        return 0;
    }
    char code[1024];

    memset(code, 0, sizeof(code));
    FILE* input_f = fopen(argv[1], "r");
    fread(code, 1, sizeof(code), input_f);
    Token tokens[strlen(code)];
    memset(tokens, 0, sizeof(tokens));
    Node* program[1024];
    Symbol* symbol_table[1024];
    memset(program, 0, sizeof(program));
    scan(code, tokens);
    parse(tokens, program, symbol_table);
    FILE* output_f = fopen(argv[2], "w");
    /*for (int i = 0; i < sizeof(program) / sizeof(Node*); i++) {
        if (NULL != program[i]) {
            print_node(output_f, program[i]);
            fprintf(output_f, "\n");
        }
    }*/
    char output[1024];
    generate(program, sizeof(program) / sizeof(Node*), output);
    fprintf(output_f, "%s", output);
    fclose(input_f);
    fclose(output_f);
    for (int i = 0; i < sizeof(program) / sizeof(Node*); i++) {
        free_node(program[i]);
    }
    for (int i = 0; i < sizeof(symbol_table) / sizeof(Symbol*); i++) {
        if (NULL != symbol_table[i]) {
            free(symbol_table[i]);
        }
    }
    return 0;
}
