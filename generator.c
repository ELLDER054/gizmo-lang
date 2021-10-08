#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"

static Stream_buf* llvm;

// llvm temporary variable name counter (I.E. %0, %1, %2, etc)
static int name_c;

// LLVM declarations

char* llvm_value(Node* n);

char* llvm_convert_type(char* t) {
    if (strcmp(t, "int") == 0) {
        return str_format("i32");
    } else if (strcmp(t, "string") == 0) {
        return str_format("i8*");
    } else if (strcmp(t, "char") == 0) {
        return str_format("i8");
    } else if (strcmp(t, "bool") == 0) {
        return str_format("i1");
    }
    return NULL;
}

char* llvm_operations(char* oper) {
    if (strcmp(oper, "+") == 0) {
        return str_format("add");
    } else if (strcmp(oper, "-") == 0) {
        return str_format("sub");
    } else if (strcmp(oper, "*") == 0) {
        return str_format("mul");
    } else if (strcmp(oper, "/") == 0) {
        return str_format("sdiv");
    }
    return NULL;
}

char* llvm_operator(Operator_node* o) {
    // To prevent recursion problems
    char* left = llvm_value(o->left);
    char* right = llvm_value(o->right);

    char* operation_name = str_format("%%%d", name_c++);
    Stream_buf_append_str(llvm, str_format("\t%s = %s %s %s, %s\n", operation_name, llvm_operations(o->oper), llvm_convert_type(type((Node*) o)), left, right));
    return operation_name;
}

char* llvm_value(Node* n) {
    if (n->n_type == INTEGER_NODE) {
        return str_format("%d", ((Integer_node*) n)->value);
    } else if (n->n_type == OPERATOR_NODE) {
        return str_format("%s", llvm_operator((Operator_node*) n));
    }
    return NULL;
}

void llvm_var_decl(Var_declaration_node* node) {
    Stream_buf_append_str(llvm, str_format("\t%%%s = alloca %s\n", node->codegen_name, llvm_convert_type(node->type)));
    Stream_buf_append_str(llvm, str_format("\tstore %s %s, %s* %%%s\n", llvm_convert_type(node->type), llvm_value(node->value), llvm_convert_type(node->type), node->codegen_name));
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
