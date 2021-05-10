#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

void generate_expression_asm(Node* n, char* code);

char* freeing[1024];
int j;

void generate_oper_asm(char* oper, Node* left, Node* right, char* c) {
    char* ret = malloc(1024);
    char* l = malloc(1024);
    char* r = malloc(1024);
    freeing[j++] = l;
    freeing[j++] = r;
    freeing[j++] = ret;
    generate_expression_asm(left, l);
    generate_expression_asm(right, r);
    switch (*oper) {
        case '+':
            strncat(c, "add ", 4);
            strcat(c, l);
            strncat(c, ", ", 2);
            strcat(c, r);
            break;
        case '-':
            strncat(c, "sub ", 4);
            strcat(c, l);
            strncat(c, ", ", 2);
            strcat(c, r);
            break;
        case '*':
            strncat(c, "mul ", 4);
            strcat(c, l);
            strncat(c, ", ", 2);
            strcat(c, r);
            break;
        case '/':
            strncat(c, "div ", 4);
            strcat(c, l);
            strncat(c, ", ", 2);
            strcat(c, r);
            break;
        default:
            break;
    }
}

void generate_expression_asm(Node* n, char* c) {
    int expr_append_c = 0;
    if (n->n_type == INTEGER_NODE) {
        char integer[100];
        snprintf(integer, 100, "%d", ((Integer_node*) n)->value);
        strcat(c, integer);
        return;
    } else if (n->n_type == ID_NODE) {
        strncat(c, "%", 1);
        strcat(c, ((Identifier_node*) n)->name);
        return;
    }
    
    generate_oper_asm(((Operator_node*) n)->oper, ((Operator_node*) n)->left, ((Operator_node*) n)->right, c);
}

void generate(Node** ast, int size, char* code) {
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char code[2056];
            strncpy(code, "%", 1);
            strcat(code, v->name);
            strncat(code, " = ", 3);
            generate_expression_asm(v->value, code);
            printf("%s\n", code);
        }
    }
    for (int i = 0; i < j; i++) {
        free(freeing[i]);
    }
}
