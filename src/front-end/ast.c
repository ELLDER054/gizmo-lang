#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"

Var_declaration_node* new_Var_declaration_node(char* type, char* codegen_name, char* name, Node* value) { /* Initializes a variable declaration node */

    Var_declaration_node* var = malloc(sizeof(Var_declaration_node));
    memset(var, 0, sizeof(Var_declaration_node));

    var->n_type = VAR_DECLARATION_NODE;

    var->name = malloc(strlen(name) + 1);
    strcpy(var->name, name);

    var->codegen_name = malloc(strlen(codegen_name) + 1);
    strcpy(var->codegen_name, codegen_name);

    var->type = malloc(strlen(type) + 1);
    strcpy(var->type, type);
    var->value = value;
    return var;
}

void free_Var_declaration_node(Var_declaration_node* n) { /* Frees a variable declaration node */
    if (n->value != NULL) {
        free_node(n->value);
    }
    free(n->name);
    free(n->codegen_name);
    free(n->type);
    free(n);
}

Var_assignment_node* new_Var_assignment_node(char* name, Node* value, char* cgid) {
    Var_assignment_node* var = malloc(sizeof(Var_assignment_node));
    memset(var, 0, sizeof(Var_assignment_node));

    var->n_type = VAR_ASSIGN_NODE;

    var->name = malloc(strlen(name) + 1);
    strcpy(var->name, name);

    var->codegen_name = malloc(strlen(cgid) + 1);
    strcpy(var->codegen_name, cgid);

    var->value = value;
    return var;
}

void free_Var_assignment_node(Var_assignment_node* v) {
    free_node(v->value);
    free(v->name);
    free(v->codegen_name);
    free(v);
}

While_loop_node* new_While_loop_node(Node* condition, Node* body, char* bcgid, char* ecgid) {
    While_loop_node* w = malloc(sizeof(While_loop_node));
    memset(w, 0, sizeof(While_loop_node));

    w->n_type = WHILE_NODE;
    w->condition = condition;
    w->body = body;

    w->begin_cgid = malloc(strlen(bcgid) + 1);
    strcpy(w->begin_cgid, bcgid);

    w->end_cgid = malloc(strlen(ecgid) + 1);
    strcpy(w->end_cgid, ecgid);

    return w;
}

void free_While_loop_node(While_loop_node* w) {
    free_node(w->condition);
    free_node(w->body);
    free(w->begin_cgid);
    free(w->end_cgid);
    free(w);
}

For_loop_node* new_For_loop_node(Node* condition, Block_node* body, char* bcgid, char* ecgid) {
    For_loop_node* f = malloc(sizeof(For_loop_node));
    memset(f, 0, sizeof(For_loop_node));

    f->n_type = FOR_NODE;
    f->condition = condition;
    f->body = body;

    f->begin_cgid = malloc(strlen(bcgid) + 1);
    strcpy(f->begin_cgid, bcgid);

    f->end_cgid = malloc(strlen(ecgid) + 1);
    strcpy(f->end_cgid, ecgid);

    return f;
}

void free_For_loop_node(For_loop_node* f) {
    free_node(f->condition);
    free_node(f->body);
    free(f->begin_cgid);
    free(f->end_cgid);
    free(f);
}

Skip_node* new_Skip_node(int kind, char* code) {
    Skip_node* skip = malloc(sizeof(Skip_node));
    memset(skip, 0, sizeof(Skip_node));

    skip->n_type = SKIP_NODE;
    skip->skip_kind = kind;

    skip->code = malloc(strlen(code) + 1);
    strcpy(skip->code, code);

    return skip;
}

void free_Skip_node(Skip_node* s) {
    free(s->code);
    free(s);
}

If_node* new_If_node(Node* condition, Node* body, Node* else_body, char* bcgid, char* ecgid, char* elcgid) {
    If_node* i = malloc(sizeof(If_node));
    memset(i, 0, sizeof(If_node));

    i->n_type = IF_NODE;
    i->condition = condition;
    i->body = body;
    i->else_body = else_body;

    i->begin_cgid = malloc(strlen(bcgid) + 1);
    strcpy(i->begin_cgid, bcgid);

    i->else_cgid = malloc(strlen(elcgid) + 1);
    strcpy(i->else_cgid, elcgid);

    i->end_cgid = malloc(strlen(ecgid) + 1);
    strcpy(i->end_cgid, ecgid);
    return i;
}

void free_If_node(If_node* i) {
    free_node(i->condition);
    free_node(i->body);
    free(i->begin_cgid);
    free(i->else_cgid);
    free(i->end_cgid);
    free(i);
}

Func_call_node* new_Func_call_node(char* name, char* type, Node** args) { /* Initializes a function call node */
    Func_call_node* func = malloc(sizeof(Func_call_node));
    memset(func, 0, sizeof(Func_call_node));
    
    if (strcmp(name, "write") == 0) {
        func->n_type = WRITE_NODE;
    } else if (strcmp(name, "read") == 0) {
        func->n_type = READ_NODE;
    } else if (strcmp(name, "len") == 0) {
        func->n_type = LEN_NODE;
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

    func->name = malloc(strlen(name) + 1);
    strcpy(func->name, name);

    func->type = malloc(strlen(type) + 1);
    strcpy(func->type, type);

    return func;
}

void free_Func_call_node(Func_call_node* f) { /* Frees a variable declaration node */
    for (int i = 0; i < f->args_len; i++) {
        free_node(f->args[i]);
    }
    free(f->args);
    free(f->name);
    free(f);
}

Operator_node* new_Operator_node(char* oper, Node* left, Node* right) { /* Initializes an operator node */
    Operator_node* op = malloc(sizeof(Operator_node));
    memset(op, 0, sizeof(Operator_node));

    op->n_type = OPERATOR_NODE;

    op->oper = malloc(strlen(oper) + 1);
    strcpy(op->oper, oper);

    op->left = left;
    op->right = right;
    return op;
}

void free_Operator_node(Operator_node* n) { /* Frees an operator node */
    free_node(n->left);
    free_node(n->right);
    free(n->oper);
    free(n);
}

Identifier_node* new_Identifier_node(char* name, char* codegen_name, char* type) { /* Initializes an identifier node */
    Identifier_node* i = malloc(sizeof(Identifier_node));
    memset(i, 0, sizeof(Identifier_node));
    
    i->n_type = ID_NODE;

    i->type = malloc(strlen(type) + 1);
    strcpy(i->type, type);

    i->name = malloc(strlen(name) + 1);
    strcpy(i->name, name);

    i->codegen_name = malloc(strlen(codegen_name) + 1);
    strcpy(i->codegen_name, codegen_name);
    return i;
}

void free_Identifier_node(Identifier_node* i) { /* Frees an identifier node */
    free(i->type);
    free(i->name);
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

Boolean_node* new_Boolean_node(int val) {
    Boolean_node* boo = malloc(sizeof(Boolean_node));
    memset(boo, 0, sizeof(Boolean_node));

    boo->n_type = BOOL_NODE;
    boo->value = val;
    return boo;
}

void free_Boolean_node(Boolean_node* b) {
    free(b);
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

    str->value = malloc(strlen(val) + 1);
    strcpy(str->value, val);

    return str;
}

void free_String_node(String_node* n) { /* Frees a string node */
    free(n->value);
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

    func->name = malloc(strlen(name) + 1);
    strcpy(func->name, name);
    
    func->type = malloc(strlen(type) + 1);
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
    free(f->name);
    free(f->type);
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
        case SKIP_NODE:
            free_Skip_node((Skip_node*) n);
            break;
        case RET_NODE:
            free_Return_node((Return_node*) n);
            break;
        case BOOL_NODE:
            free_Boolean_node((Boolean_node*) n);
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
        case WHILE_NODE:
            free_While_loop_node((While_loop_node*) n);
            break;
        case FOR_NODE:
            free_For_loop_node((For_loop_node*) n);
            break;
        case IF_NODE:
            free_If_node((If_node*) n);
            break;
        case FUNC_CALL_NODE:
        case READ_NODE:
        case WRITE_NODE:
        case LEN_NODE:
            free_Func_call_node((Func_call_node*) n);
            break;
        case FUNC_DECL_NODE:
            free_Func_decl_node((Func_decl_node*) n);
            break;
        case NODE_NODE:
            break;
    }
}
