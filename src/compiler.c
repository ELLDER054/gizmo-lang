#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "front-end/scanner.h"
#include "front-end/symbols.h"
#include "front-end/parser.h"
#include "front-end/ast.h"
#include "back-end/codegen.h"
#include "common/include/log.h"

#define GIZMO_VERSION "0.1.0"

FILE* in_file = NULL;
FILE* out_file = NULL;

void compile(char* code, char* out, char* file_name) {
    Token tokens[1024];
    memset(tokens, 0, sizeof(tokens));
    scan(code, tokens);

    Node* program[1024];
    memset(program, 0, sizeof(program));
    Symbol* symbol_table[1024];
    memset(symbol_table, 0, sizeof(symbol_table));
    parse(tokens, program, symbol_table);
    /*for (int i = 0; i < sizeof(program) / sizeof(Node*); i++) {
        if (program[i] == NULL) {
            break;
        }
        print_node(stdout, program[i]);
    }*/
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

void parse_command_line_args(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "gizmo: Expected at least an input file\n");
        exit(-1);
    }
    int has_found_in_file = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "gizmo: Expected output file after '-o'\n");
                exit(-1);
            }
            out_file = fopen(argv[++i], "w");
            if (out_file == NULL) {
                fprintf(stderr, "gizmo: Could not open output file\n");
                exit(-1);
            }
        } else if (strcmp(argv[i], "-d") == 0) {
            log_set_level(LOG_TRACE);
            log_set_quiet(0);
        } else if (argv[i][0] != '-' && !has_found_in_file) {
            in_file = fopen(argv[i], "r");
            if (in_file == NULL) {
                fprintf(stderr, "gizmo: Could not open input file");
                exit(-1);
            }
            has_found_in_file = 1;
        } else if (strcmp(argv[i], "--version") == 0) {
            fprintf(stdout, "Gizmo v%s\n", GIZMO_VERSION);
            exit(0);
        }
    }
}

int main(int argc, char** argv) {
    log_set_quiet(1);
    parse_command_line_args(argc, argv);

    char code[1024];
    memset(code, 0, sizeof(code));

    if (in_file == NULL) {
        fprintf(stderr, "gizmo: Input file not specified\n");
        exit(-1);
    }

    fread(code, 1, sizeof(code), in_file);
    log_trace("Opening source file %s\n", argv[1]);

    if (out_file == NULL) {
        out_file = fopen("a.ll", "w");
    }
    char output[4096];
    memset(output, 0, sizeof(output));
    compile(code, output, argv[1]);
    fprintf(out_file, "%s", output);
    return 0;
}
