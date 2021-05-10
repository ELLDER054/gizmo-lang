#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

void generate_expression_asm(Node* n, char* code);

char* freeing[1024];
int j;

void generate_oper_asm(char* oper, Node* left, Node* right, char* c) {
    int oper_append_c = 0;
    char* ret = malloc(sizeof(char*));
    char* l = malloc(sizeof(char*));
    char* r = malloc(sizeof(char*));
    freeing[j++] = l;
    freeing[j++] = r;
    freeing[j++] = ret;
    generate_expression_asm(left, l);
    generate_expression_asm(right, r);
    switch (*oper) {
        case '+':
            strcat(c, "add ");
            strcat(c, l);
            strcat(c, ", ");
            strcat(c, r);
            break;
        case '-':
            strcat(c, "sub ");
            strcat(c, l);
            strcat(c, ", ");
            strcat(c, r);
            break;
        case '*':
            strcat(c, "mul ");
            strcat(c, l);
            strcat(c, ", ");
            strcat(c, r);
            break;
        case '/':
            strcat(c, "div ");
            strcat(c, l);
            strcat(c, ", ");
            strcat(c, r);
            break;
        default:
            break;
    }
}

void generate_expression_asm(Node* n, char* c) {
    int expr_append_c = 0;
    if (n->n_type == INTEGER_NODE) {
        strcat(c, ((Integer_node*) n)->value);
        return;
    } else if (n->n_type == ID_NODE) {
        strcat(c, "%%");
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
            strcpy(code, "%%");
            strcat(code, v->name);
            strcat(code, " = ");
            generate_expression_asm(v->value, code);
            printf("%s\n", code);
        }
    }
    for (int i = 0; i < j; i++) {
        free(freeing[i]);
    }
}
