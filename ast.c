#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"

void print_oper(Node* n) {
    printf("%s\n", ((Operator_node*) n)->oper);
}

void print_var(Var_declaration_node* v) {
    printf("variable: %s, %s, ", v->type, v->name);
    print_oper(v->value);
}

Var_declaration_node* new_Var_declaration_node(char* type, char* name, Node* value) {

    Var_declaration_node* var = malloc(sizeof(Var_declaration_node));
    memset(var, 0, sizeof(Var_declaration_node));

    var->n_type = VAR_DECLARATION_NODE;
    strncpy(var->name, name, MAX_NAME_LEN);
    strncpy(var->type, type, MAX_TYPE_LEN);
    var->value = value;
    return var;
}

void free_Var_declaration_node(Var_declaration_node *n) {
    if (NULL != n) {
        if (NULL != n->value) {
            free_node(n->value);
        }
        free(n);
    }
}

Operator_node* new_Operator_node(char* oper, Node* left, Node* right) {
    Operator_node* op = malloc(sizeof(Operator_node));
    memset(op, 0, sizeof(Operator_node));

    op->n_type = OPERATOR_NODE;
    strncpy(op->oper, oper, MAX_OPER_LEN);
    op->left = left;
    op->right = right;
    return op;
}

void free_Operator_node(Operator_node *n) {
    free_node(n->left);
    free_node(n->right);
    free(n);
}

Integer_node* new_Integer_node(int val) {
    Integer_node* integer = malloc(sizeof(Integer_node));
    memset(integer, 0, sizeof(Integer_node));

    integer->n_type = INTEGER_NODE;
    integer->value = val;
    return integer;
}

void free_Integer_node(Integer_node *n) {
    free(n);
}

void free_node(Node* n) {
    if (NULL == n) {
        return;
    }
    switch (n->n_type) {
        case INTEGER_NODE:
            free_Integer_node((Integer_node*) n);
            break;
        case VAR_DECLARATION_NODE:
            free_Var_declaration_node((Var_declaration_node*) n);
            break;
        case OPERATOR_NODE:
            free_Operator_node((Operator_node*) n);
            break;
        case NODE_NODE:
            printf("ERROR: Should never see this!\n");
            break;
    }
}
