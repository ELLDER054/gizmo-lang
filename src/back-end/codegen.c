#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

int str_append_c = 0;

void generate(Node** ast, int size, char* code) {
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            str_append_c += snprintf(code + str_append_c, 100, "%%%s = ", v->name);
            printf("%s\n", code);
        }
    }
}
