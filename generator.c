#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"

char* global_code;

// LLVM declarations

void llvm_var_decl() {

}

// End LLVM declarations

void generate(Node** ast, char* code) {
    global_code = malloc(strlen(code) + 1);
    memcpy(global_code, code, strlen(code) + 1);

    // Current node
    Node* node;
    for (int node_c = 0; ast[node_c] != NULL; node_c++) {
        node = ast[node_c];

        Node_t node_t = node->n_type;
        if (node_t == VAR_DECLARATION_NODE) {
            llvm_var_decl();
        }
    }
}
