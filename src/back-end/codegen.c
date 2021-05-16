#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "heap.h"
#include "codegen.h"
#include "../front-end/ast.h"

#define MAX_BUF_LEN 1024

/*reminder: printf call code:

call i32 (i8*, ...)* @printf(i8* %msg, i32 12, i8 42)

*/

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
    char* l = generate_expression_asm(n->left, type, c);
    char* r = generate_expression_asm(n->right, type, c);
    char* op_name = heap_alloc(100);
    snprintf(op_name, 100, "%%%d", var_c++);
    strcat(c, op_name);
    strcat(c, " = ");
    strcat(c, find_operation_asm(n->oper));
    strcat(c, " ");
    strcat(c, type);
    strcat(c, " ");
    strcat(c, l);
    strcat(c, ", ");
    strcat(c, r);
    strcat(c, "\n");
    return op_name;
}

char* generate_expression_asm(Node* n, char* type, char* c) {
    if (n->n_type == INTEGER_NODE) {
        char number[100];
        snprintf(number, 100, "%d", ((Integer_node*) n)->value);
        char* int_name = heap_alloc(100);
        snprintf(int_name, 100, "%%%d", var_c++);
        strcat(c, int_name);
        strcat(c, " = ");
        strcat(c, number);
        strcat(c, "\n");
        return int_name;
    } else if (n->n_type == ID_NODE) {
        return ((Identifier_node*) n)->name;
    } else if (n->n_type == STRING_NODE) {
        char str[100];
        snprintf(str, 100, "`%s`", ((String_node*) n)->value);
        char* str_name = heap_alloc(100);
        snprintf(str_name, 100, "%%%d", var_c++);
        strcat(c, str_name);
        strcat(c, " = ");
        strcat(c, str);
        strcat(c, "\n");
        return str_name;
    }
    
    return generate_operation_asm((Operator_node*) n, type, c);
}

void generate(Node** ast, int size, char* code) {
    strcat(code, "define i32 @main() {\n");
    heap_init();
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char* var_name = generate_expression_asm(v->value, types(v->type), code);
            strcat(code, "%%");
            strcat(code, v->name);
            strcat(code, " = ");
            strcat(code, var_name);
            strcat(code, "\n");
        }
    }
    strcat(code, "ret i32 0\n}\n\ndeclare i32 @printf(i8* noalias nocapture, ...)\n");
    heap_free_all();
}
