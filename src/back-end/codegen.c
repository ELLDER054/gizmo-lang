#include <stdio.h>

void generate(Node** ast, char* code) {
    for (int i = 0; i < sizeof(ast) / sizeof(Node*); i++) {
        Node* n = ast[i];
        if (n->n_type == VARIABLE_NODE) {
            // add some IR to "code"
        }
    }
}
