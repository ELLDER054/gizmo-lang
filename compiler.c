#include <stdio.h>
#include <string.h>
#include "scanner.h"
#include "ast.h"

void scan(char code[1024], Token buf_toks[1024]);
void parse(Token tokens[1024], Node** program);
void print_var(Var_declaration_node* n);
void free_node(Node* n);

int main(void) {
    char code[1024];
    char c;

    memset(code, 0, sizeof(code));

    FILE* f = fopen("test.gizmo", "r");
    fread(code, 1, sizeof(code), f);
    Token tokens[strlen(code)];
    Node* program[1024];
    memset(program, 0, sizeof(program));
    scan(code, tokens);
    parse(tokens, program);
    print_var((Var_declaration_node*) program[0]);
    fclose(f);
    for (int i = 0; i < sizeof(program) / sizeof(Node*); i++) {
        free_node(program[i]);
    }
    return 0;
}
