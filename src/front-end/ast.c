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

void print_block(FILE* f, Block_node* b) {
    fprintf(f, "(BLOCK, ");
    for (int i = 0; i < sizeof(b->statements) / sizeof(Node*); i++) {
        print_node(f, b->statements[i]);
    }
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
        case BLOCK_NODE:
            print_block(f, (Block_node*) n);
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
        case READ_NODE:
        case WRITE_NODE:
            print_func_call(f, (Func_call_node*) n);
            break;
        case NODE_NODE:
            break;
    }
}

void print_var(FILE* f, Var_declaration_node* v) {
    fprintf(f, "(VAR_NODE, %s, %s, ", v->type, v->name);
    if (v->value == NULL) {
        fprintf(f, "none");
    } else {
        print_node(f, v->value);
    }
    fprintf(f, ")");
}

void print_func_call(FILE* f, Func_call_node* func) {
    fprintf(f, "(FUNC_CALL_NODE, %s, ", func->name);
    for (int i = 0; i < func->args_len; i++) {
        print_node(f, func->args[i]);
    }
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

void free_Var_declaration_node(Var_declaration_node* n) {
    if (n != NULL) {
        if (n->value != NULL) {
            free_node(n->value);
        }
        free(n);
    }
}

Func_call_node* new_Func_call_node(char* name, Node** args) {
    Func_call_node* func = malloc(sizeof(Func_call_node));
    memset(func, 0, sizeof(Func_call_node));
    
    if (strcmp(name, "write") == 0) {
        func->n_type = WRITE_NODE;
    } else if (strcmp(name, "read") == 0) {
        func->n_type = READ_NODE;
    } else {
        func->n_type = FUNC_CALL_NODE;
    }
    int len;
    for (len = 0;; len++) {
        if (args[len] == NULL) {
            break;
        }
    }
    func->args_len = len;
    func->args = malloc(len * sizeof(Node*));
    for (int i = 0; i < len; i++) {
        func->args[i] = args[i];
    }
    strncpy(func->name, name, MAX_NAME_LEN);
    return func;
}

void free_Func_call_node(Func_call_node* f) {
    if (f != NULL) {
        for (int i = 0; i < f->args_len; i++) {
            print_node(stdout, f->args[i]);
            free_node(f->args[i]);
        }
        free(f->args);
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

void free_Integer_node(Integer_node* n) {
    free(n);
}

Real_node* new_Real_node(double val) {
    Real_node* real = malloc(sizeof(Real_node));
    memset(real, 0, sizeof(Real_node));

    real->n_type = REAL_NODE;
    real->value = val;
    return real;
}

void free_Real_node(Real_node* n) {
    free(n);
}

String_node* new_String_node(char* val) {
    String_node* str = malloc(sizeof(String_node));
    memset(str, 0, sizeof(String_node));

    str->n_type = STRING_NODE;
    str->value = val;
    return str;
}

void free_String_node(String_node* n) {
    free(n);
}

Block_node* new_Block_node(Node** statements, int* ssize) {
    Block_node* block = malloc(sizeof(Block_node));
    memset(block, 0, sizeof(Block_node));
    
    block->n_type = BLOCK_NODE;
    int i;
    for (i = 0; i < ssize; i++) {
        if (statements[i] == NULL) {
            break;
        }
        block->statements[i] = statements[i];
    }
    block->ssize = ssize;
    return block;
}

void free_Block_node(Block_node* b) {
    for (int i = 0; i < b->ssize; i++) {
        free(b->statements[i]);
    }
    free(b);
}

void free_node(Node* n) {
    if (n == NULL) {
        return;
    }
    switch (n->n_type) {
        case INTEGER_NODE:
            free_Integer_node((Integer_node*) n);
            break;
        case STRING_NODE:
            free_String_node((String_node*) n);
            break;
        case BLOCK_NODE:
            free_Block_node((Block_node*) n);
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
        case READ_NODE:
        case WRITE_NODE:
            free_Func_call_node((Func_call_node*) n);
            break;
        case NODE_NODE:
            break;
    }
}
