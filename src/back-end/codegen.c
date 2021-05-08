#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

char* generate_oper_asm(char* oper, Node* left, Node* right) {
    switch (oper) {
        case '+':
            return "add %s, %s", generate_expression(left), generate_expression(right);
        case '-':
            return "sub %s, %s", generate_expression(left), generate_expression(right);
        case '*':
            return "mul %s, %s", generate_expression(left), generate_expression(right);
        case '/':
            return "div %s, %s", generate_expression(left), generate_expression(right);

void generate_expression(Node* v, char* code) {
    if (v->n_type == INTEGER_NODE) {
        char b[1024];
        snprintf(b, 1024, "%d", ((Integer_node*) v)->value);
        strncat(code, b, 1024);
        return;
    } else if (v->n_type == ID_NODE) {
        char id[1024] = "%";
        strncat(id, ((Identifier_node*) v)->name, strlen(((Identifier_node*) v)->name));
        strncat(code, id, strlen(id));
        return;
    }
    
    char* oper_asm = generate_oper_asm(((Operator_node*) v)->oper, ((Operator_node*) v)->left, ((Operator_node*) v)->right);
    strncat(code, oper_asm, strlen(oper_asm));
}

void generate(Node** ast, char* code) {
    for (int i = 0; i < sizeof(ast) / sizeof(Node*); i++) {
        Node* n = ast[i];
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char additional_code[1024] = "%";
            strncat(additional_code, v->name, strlen(v->name));
            strncat(additional_code, " = ", 3);
            int size;
            generate_expression(v->value, additional_code);
            printf("code is %s\n", additional_code);
            strncat(code, additional_code, strlen(additional_code));
        }
    }
}
