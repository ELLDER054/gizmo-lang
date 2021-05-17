#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "front-end/scanner.h"
#include "front-end/ast.h"
#include "back-end/codegen.h"

#define GIZMO_VERSION "0.1.0"

typedef struct Symbol Symbol;
void scan(char* code, Token* buf_toks);
void parse(Token* tokens, Node** program, Symbol** symbol_table);
void print_node(FILE* f, Node* n);
void free_node(Node* n);
void generate(Node** ast, int length, char* code);

int main(int argc, char** argv) {
    printf("%d, %s\n", argc, argv[1]);
    if (argc == 3 && !strcmp(argv[1], "version")) {
        fprintf(stdout, "%s", GIZMO_VERSION);
        return 0;
    } else if (!(argc == 3)) {
        fprintf(stderr, "gizmo: Expected a file to compile and a file to write output to\ngizmo: Aborting execution\n");
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
    memset(symbol_table, 0, sizeof(symbol_table));
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
        if (symbol_table[i] == NULL) {
            break;
        }
        free(symbol_table[i]);
    }
    return 0;
}
