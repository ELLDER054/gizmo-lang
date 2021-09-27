#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"

#define GIZMO_VERSION   "v0.2"

static FILE* out_file;
static FILE* in_file;
static char* in_file_name;

static void parse_command_line_args(int argc, char** argv) {
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
	parse_command_line_args(argc, argv);

    if (in_file == NULL) {
        fprintf(stderr, "gizmo: Input file not specified\n");
        exit(-1);
    }

    char c;
    Stream_buf* code = new_Stream_buf(NULL, 1);

    while ((c = fgetc(in_file)) != EOF) {
        Stream_buf_append_str(code, str_format("%c", c));
    }

    Token** tokens = malloc(code->len * sizeof(Token));
    memset(tokens, 0, code->len * sizeof(Token));
    lex((char*) (code->buf), tokens);

    Node** ast = malloc(code->len * sizeof(Node));
    memset(ast, 0, code->len * sizeof(Node));
    
    Symbol** sym_tab =  malloc(code->len * sizeof(Symbol));
    memset(sym_tab, 0, code->len * sizeof(Symbol));

    parse((char*) (code->buf), tokens, ast, sym_tab);
    return 0;
}
