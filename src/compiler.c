#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "front-end/scanner.h"
#include "front-end/ast.h"
#include "back-end/codegen.h"
#include "log.h"

typedef struct Symbol Symbol;
void scan(char* code, Token* buf_toks);
void parse(Token* tokens, Node** program, Symbol** symbol_table);
void print_node(FILE* f, Node* n);
void free_node(Node* n);
void generate(Node** ast, int length, char* code, char* file_name);

int main(int argc, char** argv) {
    log_set_level(LOG_TRACE);

    if (!(argc == 3)) {
        fprintf(stderr, "gizmo: Expected a file to compile and a file to write output to\ngizmo: Aborting execution\n");
        return -1;
    }

    char code[1024];
    memset(code, 0, sizeof(code));

    FILE* input_f = fopen(argv[1], "r");
    if (input_f == NULL) {
        fprintf(stderr, "failed to open input file");
        return -1;
    }
    log_trace("Opening source file %s\n", argv[1]);

    fread(code, 1, sizeof(code), input_f);

    Token tokens[1024];
    memset(tokens, 0, sizeof(tokens));
    scan(code, tokens);

    Node* program[1024];
    memset(program, 0, sizeof(program));
    Symbol* symbol_table[1024];
    memset(symbol_table, 0, sizeof(symbol_table));
    parse(tokens, program, symbol_table);

    FILE* output_f = fopen(argv[2], "w");
    if (output_f == NULL) {
        fprintf(stderr, "failed to open output file");
    }
    /*for (int i = 0; i < sizeof(program) / sizeof(Node*); i++) {
        if (NULL != program[i]) {
            print_node(stdout, program[i]);
            fprintf(stdout, "\n");
        }
    }*/
    char output[2048];
    memset(output, 0, sizeof(output));
    generate(program, sizeof(program) / sizeof(Node*), output, argv[1]);
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
