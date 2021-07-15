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

void print_var(FILE* f, Var_declaration_node* v);
void print_func_call(FILE* f, Func_call_node* func);

void print_node(FILE* f, Node* n) { /* Prints the given node */
    switch (n->n_type) {
        case VAR_DECLARATION_NODE:
            print_var(f, (Var_declaration_node*) n);
            break;
        case IF_NODE:
        case SKIP_NODE:
        case WHILE_NODE:
        case LIST_NODE:
        case INDEX_NODE:
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
        case VAR_ASSIGN_NODE:
            break;
        case BOOL_NODE:
            break;
        case FUNC_CALL_NODE:
        case READ_NODE:
        case WRITE_NODE:
        case LEN_NODE:
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
    free(n->codegen_name);
    free(n->name);
    free(n->type);
    free(n);
}

var_assignment_node* new_var_assignment_node(char* name, node* value, char* cgid) {
    var_assignment_node* var = malloc(sizeof(var_assignment_node));
    memset(var, 0, sizeof(var_assignment_node));

    var->n_type = var_assign_node;

    var->name = malloc(strlen(name) + 1);
    strcpy(var->name, name);

    var->codegen_name = malloc(strlen(cgid) + 1);
    strcpy(var->codegen_name, cgid);

    var->value = value;
    return var;
}

void free_var_assignment_node(var_assignment_node* v) {
    free_node(v->value);
    free(v->name);
    free(v->codegen_name);
    free(v);
}

while_loop_node* new_while_loop_node(node* condition, node* body, char* bcgid, char* ecgid) {
    while_loop_node* w = malloc(sizeof(while_loop_node));
    memset(w, 0, sizeof(while_loop_node));

    w->n_type = while_node;
    w->condition = condition;
    w->body = body;

    w->begin_cgid = malloc(strlen(bcgid) + 1);
    strcpy(w->begin_cgid, bcgid);

    w->end_cgid = malloc(strlen(ecgid) + 1);
    strcpy(w->end_cgid, ecgid);

    return w;
}

void free_while_loop_node(while_loop_node* w) {
    free_node(w->condition);
    free_node(w->body);
    free(w->begin_cgid);
    free(w->end_cgid);
    free(w);
}

skip_node* new_skip_node(int kind, char* code) {
    skip_node* skip = malloc(sizeof(skip_node));
    memset(skip, 0, sizeof(skip_node));

    skip->n_type = skip_node;
    skip->skip_kind = kind;

    skip->code = malloc(strlen(code) + 1);
    strcpy(skip->code, code);

    return skip;
}

void free_skip_node(skip_node* s) {
    free(s->code);
    free(s);
}

if_node* new_if_node(node* condition, node* body, node* else_body, char* bcgid, char* ecgid, char* elcgid) {
    if_node* i = malloc(sizeof(if_node));
    memset(i, 0, sizeof(if_node));

    i->n_type = if_node;
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

void free_if_node(if_node* i) {
    free_node(i->condition);
    free_node(i->body);
    free(i->begin_cgid);
    free(i->else_cgid);
    free(i->end_cgid);
    free(i);
}

func_call_node* new_func_call_node(char* name, node** args) { /* initializes a function call node */
    func_call_node* func = malloc(sizeof(func_call_node));
    memset(func, 0, sizeof(func_call_node));
    
    if (strcmp(name, "write") == 0) {
        func->n_type = write_node;
    } else if (strcmp(name, "read") == 0) {
        func->n_type = read_node;
    } else if (strcmp(name, "len") == 0) {
        func->n_type = len_node;
    } else {
        func->n_type = func_call_node;
    }
    int len;
    for (len = 0;; len++) {
        if (args[len] == null) {
            break;
        }
    }
    func->args_len = len;
    func->args = malloc(len * sizeof(node*));
    for (int i = 0; i < len; i++) {
        func->args[i] = args[i];
    }
    func->name = malloc(strlen(name) + 1);
    strcpy(func->name, name);
    return func;
}

void free_func_call_node(func_call_node* f) { /* frees a variable declaration node */
    for (int i = 0; i < f->args_len; i++) {
        free_node(f->args[i]);
    }
    free(f->name);
    free(f->args);
    free(f);
}

operator_node* new_operator_node(char* oper, node* left, node* right) { /* initializes an operator node */
    operator_node* op = malloc(sizeof(operator_node));
    memset(op, 0, sizeof(operator_node));

    op->n_type = operator_node;

    op->oper = malloc(strlen(oper) + 1);
    strcpy(op->oper, oper);

    op->left = left;
    op->right = right;
    return op;
}

void free_operator_node(operator_node* n) { /* frees an operator node */
    free_node(n->left);
    free_node(n->right);
    free(n->oper);
    free(n);
}

identifier_node* new_identifier_node(char* name, char* codegen_name, char* type) { /* initializes an identifier node */
    identifier_node* i = malloc(sizeof(identifier_node));
    memset(i, 0, sizeof(identifier_node));
    
    i->n_type = id_node;
    i->type = strdup(type); // continue here
    i->name = strdup(name);
    i->codegen_name = strdup(codegen_name);
    return i;
}

void free_identifier_node(identifier_node* i) { /* frees an identifier node */
    free(i->name);
    free(i->type);
    free(i->codegen_name);
    free(i);
}

integer_node* new_integer_node(int val) { /* initializes an integer node */
    integer_node* integer = malloc(sizeof(integer_node));
    memset(integer, 0, sizeof(integer_node));

    integer->n_type = integer_node;
    integer->value = val;
    return integer;
}

void free_integer_node(integer_node* n) { /* frees an integer node */
    free(n);
}

list_node* new_list_node(char* type, node** elements) {
    list_node* list = malloc(sizeof(list_node));
    memset(list, 0, sizeof(list_node));

    list->n_type = list_node;
    list->type = strdup(type);
    int len;
    for (len = 0;; len++) {
        if (elements[len] == null) {
            break;
        }
    }
    list->len = len;
    list->elements = malloc(len * sizeof(node*));
    for (int element_c = 0; element_c < len; element_c++) {
        list->elements[element_c] = elements[element_c];
    }
    return list;
}

void free_list_node(list_node* list) {
    for (int element_c = 0; element_c < list->len; element_c++) {
        free_node(list->elements[element_c]);
    }
    free(list->type);
    free(list);
}

Index_node* new_index_node(node* id, node* expr, char* type, char* cgid) {
    index_node* index = malloc(sizeof(index_node));
    memset(index, 0, sizeof(index_node));

    index->n_type = index_node;
    index->id = id;
    index->type = strdup(type);
    index->cgid = strdup(cgid);
    index->expr = expr;
    return index;
}

void free_index_node(index_node* index) {
    free_node(index->expr);
    free(index->type);
    free(index->cgid);
    free(index);
}

Boolean_node* new_boolean_node(int val) {
    boolean_node* boo = malloc(sizeof(boolean_node));
    memset(boo, 0, sizeof(boolean_node));

    boo->n_type = bool_node;
    boo->value = val;
    return boo;
}

void free_boolean_node(boolean_node* b) {
    free(b);
}

Real_node* new_real_node(double val) { /* initializes a real value node */
    real_node* real = malloc(sizeof(real_node));
    memset(real, 0, sizeof(real_node));

    real->n_type = real_node;
    real->value = val;
    return real;
}

void free_real_node(real_node* n) { /* frees a real value node */
    free(n);
}

Char_node* new_char_node(char val) { /* initializes a character node */
    char_node* ch = malloc(sizeof(char_node));
    memset(ch, 0, sizeof(char_node));

    ch->n_type = char_node;
    ch->value = val;
    return ch;
}

void free_char_node(char_node* c) { /* frees a character node */
    free(c);
}

String_node* new_string_node(char* val) { /* initializes a string node */
    string_node* str = malloc(sizeof(string_node));
    memset(str, 0, sizeof(string_node));

    str->n_type = string_node;
    str->value = strdup(val);
    return str;
}

void free_string_node(string_node* n) { /* frees a string node */
    free(n->value);
    free(n);
}

Return_node* new_return_node(node* expr) {
    return_node* ret = malloc(sizeof(return_node));
    memset(ret, 0, sizeof(return_node));

    ret->n_type = ret_node;
    ret->expr = expr;
    return ret;
}

void free_return_node(return_node* r) {
    free_node(r->expr);
    free(r);
}

Func_decl_node* new_func_decl_node(char* name, char* type, node** args, int args_len, node* body) {
    func_decl_node* func = malloc(sizeof(func_decl_node));
    memset(func, 0, sizeof(func_decl_node));

    func->n_type = func_decl_node;
    func->body = body;
    func->name = strdup(name);
    func->type = strdup(type);
    func->args_len = args_len;
    func->args = malloc(sizeof(node) * args_len);
    for (int i = 0; i < args_len; i++) {
        func->args[i] = args[i];
    }
    return func;
}

void free_func_decl_node(func_decl_node* f) {
    for (int i = 0; i < f->args_len; i++) {
        free_node(f->args[i]);
    }
    free(f->args);
    free_node(f->body);
    free(f->name);
    free(f->type);
    free(f);
}

block_node* new_block_node(node** statements, int ssize) { /* initializes a block node */
    block_node* block = malloc(sizeof(block_node));
    memset(block, 0, sizeof(block_node));
    block->statements = malloc(sizeof(node*) * ssize);
    
    block->n_type = block_node;
    int i;
    for (i = 0; i < ssize; i++) {
        if (statements[i] == null) {
            break;
        }
        block->statements[i] = statements[i];
    }
    block->ssize = ssize;
    return block;
}

void free_block_node(block_node* b) { /* frees a block node */
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
        case LIST_NODE:
            free_List_node((List_node*) n);
            break;
        case INDEX_NODE:
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
