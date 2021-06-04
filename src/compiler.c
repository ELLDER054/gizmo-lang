#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "front-end/scanner.h"
#include "front-end/symbols.h"
#include "front-end/parser.h"
#include "front-end/ast.h"
#include "back-end/codegen.h"
#include "common/include/log.h"

void compile(char* code, char* out, char* file_name) {
    Token tokens[1024];
    memset(tokens, 0, sizeof(tokens));
    scan(code, tokens);

    Node* program[1024];
    memset(program, 0, sizeof(program));
    Symbol* symbol_table[1024];
    memset(symbol_table, 0, sizeof(symbol_table));
    parse(tokens, program, symbol_table);
    #if 0
    for (int i = 0; i < sizeof(program) / sizeof(Node*); i++) {
        if (program[i] == NULL) {
            break;
        }
        print_node(stdout, program[i]);
    }
    #endif
    generate(program, sizeof(program) / sizeof(Node*), out, file_name);
    for (int i = 0; i < sizeof(program) / sizeof(Node*); i++) {
        free_node(program[i]);
    }
    for (int i = 0; i < sizeof(symbol_table) / sizeof(Symbol*); i++) {
        if (symbol_table[i] == NULL) {
            break;
        }
        free(symbol_table[i]);
    }
}

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
        fprintf(stderr, "gizmo: Failed to open input file\n");
        return -1;
    }
    fread(code, 1, sizeof(code), input_f);
    log_trace("Opening source file %s\n", argv[1]);

    FILE* output_f = fopen(argv[2], "w");
    if (output_f == NULL) {
        fprintf(stderr, "gizmo: Failed to open output file\n");
    }
    char output[2047];
    memset(output, 0, sizeof(output));
    compile(code, output, argv[1]);
    fprintf(output_f, "%s", output);
    fclose(input_f);
    fclose(output_f);
    return 0;
}
