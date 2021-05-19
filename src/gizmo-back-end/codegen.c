#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../front-end/ast.h"

void generate_variable(Var_declaration_node* var) {
    char* name = generate_expression((Node*) var->value);
    add_to_code("%%%s = %s", var->name, name);
}

void generate_func_call(char* type, Func_call_node* func) {
    char* args = generate_args_code(func->args);
    add_to_code("call %s %s(%s)", type, func->name, args);
}

void generate_statement_node(Node* n) {
    switch (n->n_type) {
        case VAR_DECLARATION_NODE:
            generate_variable((Var_declaration_node*) n);
        case WRITE_NODE:
            generate_func_call("i32 (i8*, ...)*", "@printf", (Func_call_node*) n);
    }
}

void generate(Node** program, char* code, int ast_length) {
    for (int i = 0; i < ast_length; i++) {
        Node* n = program[i];
        generate_statement_node(n);
    }
}
