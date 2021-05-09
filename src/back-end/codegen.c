#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

char* needs_freeing[1024];
int j = 0;

void generate_expression(Node* v, char* code);

char* generate_oper_asm(char* oper, Node* left, Node* right) {
    char* l = malloc(sizeof(char*));
    char* r = malloc(sizeof(char*));
    needs_freeing[j++] = l;
    needs_freeing[j++] = r;
    generate_expression(left, l);
    generate_expression(right, r);
    char* ret = malloc(sizeof(char*));
    needs_freeing[j++] = ret;
    switch (*oper) {
        case '+':
            snprintf(ret, 6 + strlen(l) + strlen(r), "add %s, %s", l, r);
            return ret;
        case '-':
            snprintf(ret, 6 + strlen(l) + strlen(r), "sub %s, %s", l, r);
            return ret;
        case '*':
            snprintf(ret, 6 + strlen(l) + strlen(r), "mul %s, %s", l, r);
            return ret;
        case '/':
            snprintf(ret, 6 + strlen(l) + strlen(r), "div %s, %s", l, r);
            return ret;
        default:
            return NULL;
    }
}

void generate_expression(Node* v, char* code) {
    if (v->n_type == INTEGER_NODE) {
        snprintf(code, 1024, "%d", ((Integer_node*) v)->value);
        return;
    } else if (v->n_type == ID_NODE) {
        char id[1024] = "%";
        snprintf(code, sizeof(((Identifier_node*) v)->name), "%s", ((Identifier_node*) v)->name);
        return;
    }
    
    char* oper_asm = generate_oper_asm(((Operator_node*) v)->oper, ((Operator_node*) v)->left, ((Operator_node*) v)->right);
    snprintf(code, sizeof(oper_asm), "%s", oper_asm);
}

void generate(Node** ast, int size, char* code) {
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char additional_code[1024] = "%";
            snprintf(additional_code, sizeof(v->name), "%s", v->name);
            snprintf(additional_code, 3, " = ");
            int size;
            generate_expression(v->value, additional_code);
            printf("code is %s\n", additional_code);
            strncat(code, additional_code, strlen(additional_code));
        }
    }
    for (int i = 0; i < sizeof(needs_freeing) / sizeof(char*); i++) {
        free(needs_freeing[i]);
    }
}
