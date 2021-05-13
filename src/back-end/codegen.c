#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

#define MAX_BUF_LEN 1024

void generate_expression_asm(Node* n, char* code);

char* freeing[MAX_BUF_LEN];
int j = 0;

void generate_oper_asm(char* oper, Node* left, Node* right, char* c) {
    char* ret = malloc(MAX_BUF_LEN);
    memset(ret, 0, MAX_BUF_LEN);
    char* l = malloc(MAX_BUF_LEN);
    memset(l, 0, MAX_BUF_LEN);
    char* r = malloc(MAX_BUF_LEN);
    memset(r, 0, MAX_BUF_LEN);
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
            strcat(c, "\n");
            break;
        case '-':
            strcat(c, "sub ");
            strcat(c, l);
            strcat(c, ", ");
            strcat(c, r);
            strcat(c, "\n");
            break;
        case '*':
            strcat(c, "mul ");
            strcat(c, l);
            strcat(c, ", ");
            strcat(c, r);
            strcat(c, "\n");
            break;
        case '/':
            strcat(c, "div ");
            strcat(c, l);
            strcat(c, ", ");
            strcat(c, r);
            strcat(c, "\n");
            break;
        default:
            break;
    }
}

void generate_expression_asm(Node* n, char* c) {
    int expr_append_c = 0;
    if (n->n_type == INTEGER_NODE) {
        char integer[100];
        snprintf(integer, 100, "add i32 %d, 0", ((Integer_node*) n)->value);
        strcat(c, integer);
        return;
    } else if (n->n_type == STRING_NODE) {
        char str[100];
        snprintf(str, 100, "\"%s\"", ((String_node*) n)->value); /* strings are not generated correctly */
        strcat(c, str);
        return;
    } else if (n->n_type == ID_NODE) {
        strcat(c, "%");
        strcat(c, ((Identifier_node*) n)->name);
        return;
    }
    
    generate_oper_asm(((Operator_node*) n)->oper, ((Operator_node*) n)->left, ((Operator_node*) n)->right, c);
}

char* types(char* t) {
    if (!strcmp(t, "int")) {
        return "i32";
    }
    return "";
}

void generate(Node** ast, int size, char* code) {
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            strcat(code, "%");
            strcat(code, v->name);
            strcat(code, " = ");
            generate_expression_asm(v->value, code);
            strcat(code, "\n");
        }
    }
    for (int i = 0; i < j; i++) {
        free(freeing[i]);
    }
}
