#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"

void print_node(FILE* f, Node* n);

void print_oper(FILE* f, Operator_node* n) {
    fprintf(f, "(OPER_NODE, %s, ", n->oper);
    print_node(f, n->left);
    fprintf(f, ", ");
    print_node(f, n->right);
    fprintf(f, ")");
}

void print_id(FILE* f, Identifier_node* n) {
    fprintf(f, "(ID_NODE, %s)", n->name);
}

void print_int(FILE* f, Integer_node* i) {
    fprintf(f, "(INT_NODE, %d)", i->value);
}

void print_real(FILE* f, Real_node* r) {
    fprintf(f, "(REAL_NODE, %f)", r->value);
}

void print_str(FILE* f, String_node* s) {
    fprintf(f, "(STR_NODE, %s)", s->value);
}

void print_var(FILE* f, Var_declaration_node* v);
void print_func_call(FILE* f, Func_call_node* func);

void print_node(FILE* f, Node* n) {
    switch (n->n_type) {
        case VAR_DECLARATION_NODE:
            print_var(f, (Var_declaration_node*) n);
            break;
        case OPERATOR_NODE:
            print_oper(f, (Operator_node*) n);
            break;
        case INTEGER_NODE:
            print_int(f, (Integer_node*) n);
            break;
        case STRING_NODE:
            print_str(f, (String_node*) n);
            break;
        case ID_NODE:
            print_id(f, (Identifier_node*) n);
            break;
        case REAL_NODE:
            print_real(f, (Real_node*) n);
            break;
        case FUNC_CALL_NODE:
            print_func(f, (Func_call_node*) n);
            break;
        case NODE_NODE:
            break;
    }
}

void print_var(FILE* f, Var_declaration_node* v) {
    fprintf(f, "(VAR_NODE, %s, %s, ", v->type, v->name);
    if (NULL == v->value) {
        fprintf(f, "NULL");
    } else {
        print_node(f, v->value);
    }
    fprintf(f, ")");
}

void print_var(FILE* f, Func_call_node* func) {
    fprintf(f, "(FUNC_CALL_NODE, %s, ", func->name);
    fprintf(f, ")");
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

Func_call_node* new_Func_call_node(char* name, Node** args) {

    Func_call_node* func = malloc(sizeof(Func_call_node));
    memset(func, 0, sizeof(Func_call_node));

    func->n_type = FUNC_CALL_NODE;
    strncpy(func->name, name, MAX_NAME_LEN);
    func->args = args;
    return func;
}

void free_Func_call_node(Func_call_node *f) {
    if (NULL != f) {
        for (int i = 0; i < sizeof(f->args) / sizeof(Arg*); i++) {
            free(f->args[i]);
        }
        free(f);
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

void free_Operator_node(Operator_node* n) {
    free_node(n->left);
    free_node(n->right);
    free(n);
}

Identifier_node* new_Identifier_node(char* name) {
    Identifier_node* i = malloc(sizeof(Identifier_node));
    memset(i, 0, sizeof(Identifier_node));
    
    i->n_type = ID_NODE;
    strncpy(i->name, name, MAX_NAME_LEN);
    return i;
}

void free_Identifier_node(Identifier_node* i) {
    free(i);
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

Real_node* new_Real_node(double val) {
    Real_node* real = malloc(sizeof(Real_node));
    memset(real, 0, sizeof(Real_node));

    real->n_type = REAL_NODE;
    real->value = val;
    return real;
}

void free_Real_node(Real_node *n) {
    free(n);
}

String_node* new_String_node(char* val) {
    String_node* str = malloc(sizeof(String_node));
    memset(str, 0, sizeof(String_node));

    str->n_type = STRING_NODE;
    str->value = val;
    return str;
}

void free_String_node(String_node *n) {
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
        case STRING_NODE:
            free_String_node((String_node*) n);
            break;
        case REAL_NODE:
            free_Real_node((Real_node*) n);
            break;
        case VAR_DECLARATION_NODE:
            free_Var_declaration_node((Var_declaration_node*) n);
            break;
        case OPERATOR_NODE:
            free_Operator_node((Operator_node*) n);
            break;
        case ID_NODE:
            free_Identifier_node((Identifier_node*) n);
            break;
        case FUNC_CALL_NODE:
            free_Func_call_node((Func_call_node*) n);
            break;
        case NODE_NODE:
            break;
    }
}
