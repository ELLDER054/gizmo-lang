#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"

void print_oper(Node* n) {
    printf(".%c.\n", *((Operator_node*) &n)->oper);
}

void print_var(Var_declaration_node* v) {
    printf("%s, %s\n", v->type, v->name);
    printf("%d\n", v->value->n_type);
    print_oper(v->value);
}

Var_declaration_node* new_Var_declaration_node(char* type, char* name, Node* value) {
      printf("about to malloc 2\n");

    Var_declaration_node* var = malloc(sizeof(Var_declaration_node));
        printf("done to malloc 2\n");

    strncpy(var->name, name, MAX_NAME_LEN);
    strncpy(var->type, type, MAX_TYPE_LEN);
    var->value = value;
    return var;
}

void free_Var_declaration_node(Var_declaration_node *n) {
    free(n);
}

Operator_node* new_Operator_node(char* oper, Node* left, Node* right) {
    printf("about to malloc\n");
    Operator_node* op = malloc(sizeof(Operator_node));
        printf("done to malloc\n");

    strncpy(op->oper, oper, MAX_NAME_LEN);
    op->left = left;
    op->right = right;
    return op;
}

void free_Operator_node(Operator_node *n) {
    free(n);
}

Integer_node* new_Integer_node(int val) {
    printf("about to malloc 3\n");
    Integer_node* integer = malloc(sizeof(Integer_node));
        printf("done to malloc 3\n");

    integer->value = val;
    return integer;
}

void free_Integer_node(Integer_node *n) {
    free(n);
}