#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "codegen.h"
#include "../front-end/ast.h"

#define MAX_BUF_LEN 1024

/*reminder: printf code:

declare i32 @printf(i8* noalias nocapture, ...)
call i32 (i8*, ...)* @printf(i8* %msg, i32 12, i8 42)

*/

char* freeing[MAX_BUF_LEN];
memset(freeing, 0, MAX_BUF_LEN);
int j = 0;
int var_c = 0;

char* types(char* t) {
    if (!strcmp(t, "int")) {
        return "i32";
    }
    return "";
}

char* find_operation_asm(char* oper) {
    switch (*oper) {
        case '+':
            return "add";
        case '-':
            return "sub";
        case '*':
            return "mul";
        case '/':
            return "div";
    }
    return "";
}

char* generate_expression_asm(Node* n, char* type, char* c);

char* generate_operation_asm(Operator_node* n, char* type, char* c) {
    char fake_buffer[MAX_BUF_LEN];
    char* l = generate_expression_asm(n->left, type, fake_buffer);
    char* r = generate_expression_asm(n->right, type, fake_buffer);
    memset(l, 0, MAX_BUF_LEN);
    memset(r, 0, MAX_BUF_LEN);
    freeing[j++] = l;
    freeing[j++] = r;
    char* name = malloc(100);
    freeing[j++] = name;
    memset(name, 0, 100);
    snprintf(name, 100, "%%%d", var_c++);
    strcat(c, name);
    strcat(c, " = ");
    strcat(c, find_operation_asm(n->oper));
    strcat(c, " ");
    strcat(c, type);
    strcat(c, " ");
    strcat(c, l);
    strcat(c, ", ");
    strcat(c, r);
    strcat(c, "\n");
    return name;
}

char* generate_expression_asm(Node* n, char* type, char* c) {
    if (n->n_type == INTEGER_NODE) {
        char number[100];
        snprintf(number, 100, "%d", ((Integer_node*) n)->value);
        char* name = malloc(100);
        memset(name, 0, 100);
        freeing[j++] = name;
        snprintf(name, 100, "%%%d", var_c++);
        strcat(c, name);
        strcat(c, " = ");
        strcat(c, number);
        strcat(c, "\n");
        return name;
    } else if (n->n_type == ID_NODE) {
        return ((Identifier_node*) n)->name;
    } else if (n->n_type == STRING_NODE) {
        char str[100];
        memset(str, 0, 100);
        snprintf(str, 100, "`%s`", ((String_node*) n)->value);
        char* name = malloc(100);
        memset(name, 0, 100);
        freeing[j++] = name;
        snprintf(name, 100, "%%%d", var_c++);
        strcat(c, name);
        strcat(c, " = ");
        strcat(c, str);
        strcat(c, "\n");
        return name;
    }
    
    return generate_operation_asm((Operator_node*) n, type, c);
}

void generate(Node** ast, int size, char* code) {
    strcat(code, "define i32 @main() {\n");
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char* name = generate_expression_asm(v->value, v->type, code);
            strcat(code, "%");
            strcat(code, v->name);
            strcat(code, " = ");
            strcat(code, name);
            strcat(code, "\n");
        }
    }
    strcat(code, "ret i32 0\n}");
    for (int i = 0; i < j; i++) {
        free(freeing[i]);
    }
}
