#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "heap.h"
#include "codegen.h"
#include "../front-end/ast.h"

#define MAX_BUF_LEN 1024

int var_c = 1;
int str_c = 1;
char* type(Node* n);

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
        strcat(c, " = add i32 0, ");
        strcat(c, number);
        strcat(c, "\n");
        return int_name;
    } else if (n->n_type == ID_NODE) {
        char* id_name = heap_alloc(100);
        snprintf(id_name, 100, "%%%s", ((Identifier_node*) n)->name);
        return id_name;
    } else if (n->n_type == STRING_NODE) {
        char str[100];
        snprintf(str, 100, "%s", ((String_node*) n)->value);
        char* str_name = heap_alloc(100);
        snprintf(str_name, 100, "%%%d", var_c++);
        strcat(c, str_name);
        strcat(c, " = alloca i8*, align 8\n");
        strcat(c, "store i8* getelementptr inbounds ([");
        char* len = heap_alloc(100);
        itoa(strlen(str), len, 10);
        strcat(c, len);
        strcat(c, " x i8], [");
        strcat(c, len);
        strcat(c, " x i8]* @.str.");
        char* s_c = heap_alloc(100);
        itoa(str_c, s_c, 10);
        strcat(c, s_c);
        strcat(c, ", i32 0, i32 0) i8** ");
        strcat(c, str_name);
        strcat(c, ", align 8");
        return str_name;
    }
    
    return generate_operation_asm((Operator_node*) n, type, c);
}

void generate(Node** ast, int size, char* code) {
    strcat(code, "@num1 = private unnamed_addr constant [3 x i8] c\"%d\\00\"\n\ndefine i32 @main() {\n");
    heap_init();
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        printf("%d\n", n->n_type);
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char* var_name = generate_expression_asm(v->value, types(v->type), code);
            strcat(code, "%");
            strcat(code, v->name);
            strcat(code, " = add i32 0, ");
            strcat(code, var_name);
            strcat(code, "\n");
        } else if (n->n_type == WRITE_NODE) {
            Func_call_node* func = (Func_call_node*) n;
            char* write_arg_name = generate_expression_asm(func->args[0], types(type(func->args[0])), code);
            if (!strcmp(type(func->args[0]), "int")) {
                strcat(code, "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @num1, i32 0, i32 0), i32 ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            }
            strcat(code, "\n");
        }
    }
    strcat(code, "ret i32 0\n}\n\ndeclare i32 @printf(i8* noalias nocapture, ...)\n");
    heap_free_all();
}
