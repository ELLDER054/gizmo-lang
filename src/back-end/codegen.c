#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

int str_append_c = 0;

char* generate_expression_asm(Node* n) {
    char code[1024];
    if (n->n_type == INTEGER_NODE) {
        str_append_c += snprintf(code + str_append_c, 100, "%d", ((Integer_node*) n)->value);
        return code;
    } else if (n->n_type == ID_NODE) {
        str_append_c += snprintf(code + str_append_c, 100, "%%%s", ((Identifier_node*) n)->name);
        return code;
    }
    return generate_oper_asm();
}

void generate(Node** ast, int size, char* code) {
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            str_append_c += snprintf(code + str_append_c, 100, "%%%s = %s", v->name, generate_expression_asm(v->value));
            printf("%s\n", code);
        }
    }
}
