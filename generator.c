#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"

static Stream_buf* llvm;

// llvm temporary variable name counter (I.E. %0 or %6)
static int name_c;

// LLVM declarations

void llvm_var_decl(Var_declaration_node* node) {
    Stream_buf_append_str(llvm, str_format("%%%s = call i32 @malloc(i32 %d)\n", node->codegen_name, 5));
}

// End LLVM declarations

void generate(Node** ast, char* code) {
    llvm = new_Stream_buf(NULL, 1);

    // Current node
    Node* node;
    for (int node_c = 0; ast[node_c] != NULL; node_c++) {
        node = ast[node_c];

        Node_t node_t = node->n_type;
        if (node_t == VAR_DECLARATION_NODE) {
            llvm_var_decl((Var_declaration_node*) node);
        }
    }
    memcpy(code, llvm->buf, strlen((char*) (llvm->buf)));
}
