#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../front-end/ast.h"
#include "../back-end/heap.h"

char* generate_operation(Operator_node* value) {
    char* left_expr = generate_expression(value->left);
    char* right_expr = generate_expression(value->right);
    add_to_code();
}

char* generate_expression(Node* value) {
    if (value->n_type == INTEGER_NODE) {
        char* name = heap_alloc(1024);
        sprintf(name, "%%%d", var_c++);
        add_to_code("%%%s = %d", name, ((Integer_node*) value)->value);
        return name;
    } else if (value->n_type == STRING_NODE) {
        char* name = heap_alloc(1024);
        insert_before_code("@.str.%d = private unnamed_addr constant [%d x i8] c%s", str_c++, strlen(((String_node*) value)->value), ((String_node*) value)->value);
        sprintf(name, "%%%d", var_c++);
        add_to_code("%%%s = %d", name, ((Integer_node*) value)->value);
        return name;
    } else if (value->n_type == REAL_NODE) {
        char* name = heap_alloc(1024);
        sprintf(name, "%%%d", var_c++);
        add_to_code("%%%s = alloca double, align 8\nstore double %d, %%%s", name, ((Real_node*) value)->value, name);
        return name;
    }
    
    return generate_operation((Operator_node*) value);
}

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
