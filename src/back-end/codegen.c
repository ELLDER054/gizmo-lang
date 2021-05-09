#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

void generate_expression_asm(Node* n, char* code);

int str_append_c = 0;
freeing[1024];
int j;

char* generate_oper_asm(char* oper, Node* left, Node* right, char* code) {
    char* ret = malloc(sizeof(char*));
    char* l = malloc(sizeof(char*));
    char* r = malloc(sizeof(char*));
    generate_expression_asm(left, l);
    generate_expression_asm(right, r);
    switch (*oper) {
        case '+':
            str_append_c += snprintf(code + str_append_c, 100, "add %s, %s", (l, r);
            break;
        case '-':
            str_append_c += snprintf(code + str_append_c, 100, "sub %s, %s", (l, r));
            break;
        case '*':
            str_append_c += snprintf(code + str_append_c, 100, "mul %s, %s", (l, r);
            break;
        case '/':
            str_append_c += snprintf(code + str_append_c, 100, "div %s, %s", (l, r);
            break;
        default:
            break;
    }
}

void generate_expression_asm(Node* n, char* code) {
    if (n->n_type == INTEGER_NODE) {
        str_append_c += snprintf(code + str_append_c, 100, "%d", ((Integer_node*) n)->value);
        return;
    } else if (n->n_type == ID_NODE) {
        str_append_c += snprintf(code + str_append_c, 100, "%%%s", ((Identifier_node*) n)->name);
        return;
    }
    
    generate_oper_asm(((Operator_node*) n)->oper, ((Operator_node*) n)->left, ((Operator_node*) n)->right, code);
}

void generate(Node** ast, int size, char* code) {
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char code[1024];
            generate_expression_asm(v->value, code);
            str_append_c += snprintf(code + str_append_c, 100, "%%%s = %s", v->name, code);
            printf("%s\n", code);
        }
    }
    for (int i = 0; i < j; i++) {
        free(freeing[i]);
    }
}
