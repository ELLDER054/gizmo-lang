#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"

void print_node(FILE* f, Node* n);

void print_func_decl(FILE* f, Func_decl_node* func) {
    fprintf(f, "(FUNC_DECL, %s, %s)", func->name, func->type);
}

void print_return(FILE* f, Return_node* r) {
    fprintf(f, "(RET, ");
    print_node(f, r->expr);
    fprintf(f, ")");
}

void print_oper(FILE* f, Operator_node* n) { /* Prints an operator node */
    fprintf(f, "(OPER_NODE, %s, ", n->oper);
    print_node(f, n->left);
    fprintf(f, ", ");
    print_node(f, n->right);
    fprintf(f, ")");
}

void print_block(FILE* f, Block_node* b) { /* Prints a block node */
    fprintf(f, "(BLOCK, ");
    for (int i = 0; i < b->ssize; i++) {
        print_node(f, b->statements[i]);
    }
    fprintf(f, ")");
}

void print_id(FILE* f, Identifier_node* n) { /* Prints an identifier node */
    fprintf(f, "(ID_NODE, %s)", n->name);
}

void print_int(FILE* f, Integer_node* i) { /* Prints an integer node */
    fprintf(f, "(INT_NODE, %d)", i->value);
}

void print_real(FILE* f, Real_node* r) { /* Prints a real value node */
    fprintf(f, "(REAL_NODE, %f)", r->value);
}

void print_char(FILE* f, Char_node* c) { /* Prints a character node */
    fprintf(f, "(CHAR_NODE, %c)", c->value);
}

void print_str(FILE* f, String_node* s) { /* Prints a string node */
    fprintf(f, "(STR_NODE, %s)", s->value);
}

void print_var_assign(FILE* f, Var_assignment_node* n) {
    fprintf(f, "(VA, %s)", n->name);
}

void print_var(FILE* f, Var_declaration_node* v);
void print_func_call(FILE* f, Func_call_node* func);

void print_node(FILE* f, Node* n) { /* Prints the given node */
    switch (n->n_type) {
        case VAR_DECLARATION_NODE:
            print_var(f, (Var_declaration_node*) n);
            break;
        case VAR_ASSIGN_NODE:
            print_var_assign(f, (Var_assignment_node*) n);
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
        case CHAR_NODE:
            print_char(f, (Char_node*) n);
            break;
        case RET_NODE:
            print_return(f, (Return_node*) n);
            break;
        case REAL_NODE:
            print_real(f, (Real_node*) n);
            break;
        case FUNC_CALL_NODE:
        case READ_NODE:
        case WRITE_NODE:
            print_func_call(f, (Func_call_node*) n);
            break;
        case FUNC_DECL_NODE:
            print_func_decl(f, (Func_decl_node*) n);
        case NODE_NODE:
            break;
    }
}

void print_var(FILE* f, Var_declaration_node* v) { /* Prints a variable node */
    fprintf(f, "(VAR_NODE, %s, %s, ", v->type, v->name);
    if (v->value == NULL) {
        fprintf(f, "none");
    } else {
        print_node(f, v->value);
    }
    fprintf(f, ")");
}

void print_func_call(FILE* f, Func_call_node* func) { /* Prints a function call node */
    fprintf(f, "(FUNC_CALL_NODE, %s, ", func->name);
    for (int i = 0; i < func->args_len; i++) {
        print_node(f, func->args[i]);
    }
    fprintf(f, ")");
}

Var_declaration_node* new_Var_declaration_node(char* type, char* codegen_name, char* name, Node* value) { /* Initializes a variable declaration node */

    Var_declaration_node* var = malloc(sizeof(Var_declaration_node));
    memset(var, 0, sizeof(Var_declaration_node));

    var->n_type = VAR_DECLARATION_NODE;
    strncpy(var->name, name, MAX_NAME_LEN);
    strncpy(var->codegen_name, codegen_name, MAX_NAME_LEN + 4);
    strncpy(var->type, type, MAX_TYPE_LEN);
    var->value = value;
    return var;
}

void free_Var_declaration_node(Var_declaration_node* n) { /* Frees a variable declaration node */
    if (n != NULL) {
        if (n->value != NULL) {
            free_node(n->value);
        }
        free(n);
    }
}

Var_assignment_node* new_Var_assignment_node(char* name, char* codegen_name, Node* value) {
    Var_assignment_node* var = malloc(sizeof(Var_assignment_node));
    memset(var, 0, sizeof(Var_assignment_node));

    var->n_type = VAR_ASSIGN_NODE;
    strncpy(var->name, name, MAX_NAME_LEN);
    strncpy(var->codegen_name, codegen_name, MAX_NAME_LEN + 4);
    var->value = value;
    return var;
}

void free_Var_assignment_node(Var_assignment_node* v) {
    if (v != NULL) {
        if (v->value != NULL) {
            free_node(v->value);
        }
        free(v);
    }
}

Func_call_node* new_Func_call_node(char* name, Node** args) { /* Initializes a function call node */
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

void free_Func_call_node(Func_call_node* f) { /* Frees a variable declaration node */
    if (f != NULL) {
        for (int i = 0; i < f->args_len; i++) {
            free_node(f->args[i]);
        }
        free(f->args);
        free(f);
    }
}

Operator_node* new_Operator_node(char* oper, Node* left, Node* right) { /* Initializes an operator node */
    Operator_node* op = malloc(sizeof(Operator_node));
    memset(op, 0, sizeof(Operator_node));

    op->n_type = OPERATOR_NODE;
    strncpy(op->oper, oper, MAX_OPER_LEN);
    op->left = left;
    op->right = right;
    return op;
}

void free_Operator_node(Operator_node* n) { /* Frees an operator node */
    free_node(n->left);
    free_node(n->right);
    free(n);
}

Identifier_node* new_Identifier_node(char* name, char* codegen_name, char* type) { /* Initializes an identifier node */
    Identifier_node* i = malloc(sizeof(Identifier_node));
    memset(i, 0, sizeof(Identifier_node));
    
    i->n_type = ID_NODE;
    strncpy(i->type, type, MAX_TYPE_LEN);
    strncpy(i->name, name, MAX_NAME_LEN);
    strncpy(i->codegen_name, codegen_name, MAX_NAME_LEN + 4);
    return i;
}

void free_Identifier_node(Identifier_node* i) { /* Frees an identifier node */
    free(i);
}

Integer_node* new_Integer_node(int val) { /* Initializes an integer node */
    Integer_node* integer = malloc(sizeof(Integer_node));
    memset(integer, 0, sizeof(Integer_node));

    integer->n_type = INTEGER_NODE;
    integer->value = val;
    return integer;
}

void free_Integer_node(Integer_node* n) { /* Frees an integer node */
    free(n);
}

Real_node* new_Real_node(double val) { /* Initializes a real value node */
    Real_node* real = malloc(sizeof(Real_node));
    memset(real, 0, sizeof(Real_node));

    real->n_type = REAL_NODE;
    real->value = val;
    return real;
}

void free_Real_node(Real_node* n) { /* Frees a real value node */
    free(n);
}

Char_node* new_Char_node(char val) { /* Initializes a character node */
    Char_node* ch = malloc(sizeof(Char_node));
    memset(ch, 0, sizeof(Char_node));

    ch->n_type = CHAR_NODE;
    ch->value = val;
    return ch;
}

void free_Char_node(Char_node* c) { /* Frees a character node */
    free(c);
}

String_node* new_String_node(char* val) { /* Initializes a string node */
    String_node* str = malloc(sizeof(String_node));
    memset(str, 0, sizeof(String_node));

    str->n_type = STRING_NODE;
    str->value = val;
    return str;
}

void free_String_node(String_node* n) { /* Frees a string node */
    free(n);
}

Return_node* new_Return_node(Node* expr) {
    Return_node* ret = malloc(sizeof(Return_node));
    memset(ret, 0, sizeof(Return_node));

    ret->n_type = RET_NODE;
    ret->expr = expr;
    return ret;
}

void free_Return_node(Return_node* r) {
    free_node(r->expr);
    free(r);
}

Func_decl_node* new_Func_decl_node(char* name, char* type, Node** args, int args_len, Node* body) {
    Func_decl_node* func = malloc(sizeof(Func_decl_node));
    memset(func, 0, sizeof(Func_decl_node));

    func->n_type = FUNC_DECL_NODE;
    func->body = body;
    strcpy(func->name, name);
    strcpy(func->type, type);
    func->args_len = args_len;
    func->args = malloc(sizeof(Node) * args_len);
    for (int i = 0; i < args_len; i++) {
        func->args[i] = args[i];
    }
    return func;
}

void free_Func_decl_node(Func_decl_node* f) {
    for (int i = 0; i < f->args_len; i++) {
        free_node(f->args[i]);
    }
    free(f->args);
    free_node(f->body);
    free(f);
}

Block_node* new_Block_node(Node** statements, int ssize) { /* Initializes a block node */
    Block_node* block = malloc(sizeof(Block_node));
    memset(block, 0, sizeof(Block_node));
    block->statements = malloc(sizeof(Node*) * ssize);
    
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

void free_Block_node(Block_node* b) { /* Frees a block node */
    for (int i = 0; i < b->ssize; i++) {
        free_node(b->statements[i]);
    }
    free(b->statements);
    free(b);
}

void free_node(Node* n) { /* Frees the given node */
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
        case RET_NODE:
            free_Return_node((Return_node*) n);
            break;
        case CHAR_NODE:
            free_Char_node((Char_node*) n);
            break;
        case BLOCK_NODE:
            free_Block_node((Block_node*) n);
            break;
        case REAL_NODE:
            free_Real_node((Real_node*) n);
            break;
        case VAR_DECLARATION_NODE:
            free_Var_declaration_node((Var_declaration_node*) n);
            break;
        case VAR_ASSIGN_NODE:
            free_Var_assignment_node((Var_assignment_node*) n);
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
        case FUNC_DECL_NODE:
            free_Func_decl_node((Func_decl_node*) n);
            break;
        case NODE_NODE:
            break;
    }
}
