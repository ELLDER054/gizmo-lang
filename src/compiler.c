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
char* in_file_name = NULL;

void compile(char* code, char* out, char* file_name) {
    Token* tokens = malloc(strlen(code) * sizeof(Token));
    memset(tokens, 0, strlen(code) * sizeof(Token));
    scan(code, tokens);

    Node** program = malloc(strlen(code) * sizeof(Node*));
    memset(program, 0, strlen(code) * sizeof(Node*));
    Symbol** symbol_table = malloc(strlen(code) * sizeof(Symbol*));
    memset(symbol_table, 0, strlen(code) * sizeof(Symbol*));
    parse(code, tokens, program, symbol_table);

    generate(program, strlen(code) * sizeof(Node*), out, file_name);
    for (int i = 0; i < strlen(code); i++) {
        free_node(program[i]);
    }
    for (int i = 0; i < strlen(code); i++) {
        if (symbol_table[i] != NULL) {
            free(symbol_table[i]);
        }
    }
    free(program);
    free(symbol_table);
    free(tokens);
}

void parse_command_line_args(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stdout, "Gizmo, a fast and user-friendly programming language for parsing and manipulating strings\n");
        exit(-1);
    }
    int has_found_in_file = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "gizmo: Expected output file after '-o'\n");
                exit(-1);
            }
            int index = ++i;
            out_file = fopen(argv[index], "w");
            if (out_file == NULL) {
                fprintf(stderr, "gizmo: Could not open output file\n");
                exit(-1);
            }
        } else if (strcmp(argv[i], "-d") == 0) {
            log_set_level(LOG_TRACE);
            log_set_quiet(0);
        } else if (argv[i][0] != '-' && !has_found_in_file) {
            in_file = fopen(argv[i], "r");
            in_file_name = malloc(strlen(argv[i]) + 1);
            memset(in_file_name, 0, strlen(argv[i]) + 1);
            sprintf(in_file_name, "%s", argv[i]);
            if (in_file == NULL) {
                fprintf(stderr, "gizmo: Could not open input file\n");
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

    if (in_file == NULL) {
        fprintf(stderr, "gizmo: Input file not specified\n");
        exit(-1);
    }
    
    char c;
    int len = 0;
	while ((c = fgetc(in_file)) != EOF) {
    	len++;
	}
    char* code = malloc(len + 1);
    memset(code, 0, len + 1);
    
    rewind(in_file);
	while ((c = fgetc(in_file)) != EOF) {
    	strncat(code, &c, 1);
	}

    log_trace("Opening source file %s\n", argv[1]);

    if (out_file == NULL) {
        out_file = fopen("a.ll", "w");
    }
    char* output = malloc(10000);
    memset(output, 0, 10000);

    compile(code, output, in_file_name);

    fprintf(out_file, "%s", output);
    free(in_file_name);
    free(output);
    free(code);
    return 0;
}
