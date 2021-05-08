#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

void generate_expression(Node* v, char* code) {
    if (v->n_type == INTEGER_NODE) {
        char b[1024];
        itoa(((Integer_node*) v)->value, b, 10);
        strcat(code, b);
        return;
    }
}

void generate(Node** ast, char* code) {
    for (int i = 0; i < sizeof(ast) / sizeof(Node*); i++) {
        Node* n = ast[i];
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char additional_code[1024] = "%%%s = ";
            int size;
            generate_expression(v->value, additional_code);
            printf("code is %s\n", additional_code);
            strncat(code, additional_code, strlen(additional_code));
        }
    }
}
