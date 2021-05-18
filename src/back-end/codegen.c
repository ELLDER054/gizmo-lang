#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "heap.h"
#include "codegen.h"
#include "str_tracker.h"
#include "../front-end/ast.h"

#define MAX_BUF_LEN 1024

int var_c = 1;
int str_c = 1;
char* type(Node* n);
ht* str_tracker;

char* types(char* t) {
    if (!strcmp(t, "int")) {
        return "i32";
    } else if (!strcmp(t, "real")) {
        return "double";
    }
    return "";
}

void insert(char* buf, int pos, int size, char* str) {
    char temp[MAX_BUF_LEN * 2];
    for (int i = 0; i < pos; i++) {
        strncat(temp, &buf[i], 1);
    }
    strcat(temp, str);
    for (int i = pos; i < size; i++) {
        strncat(temp, &buf[i], 1);
    }
    strcpy(buf, temp);
}

char* find_operation_asm(char* oper, char* t) {
    printf("type: %s\n", t);
    if (!strcmp(t, "int")) {
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
    } else if (!strcmp(t, "double")) {
        switch (*oper) {
            case '+':
                return "fadd";
            case '-':
                return "fsub";
            case '*':
                return "fmul";
            case '/':
                return "fdiv";
        }
    }
    return "";
}

char* generate_expression_asm(Node* n, char* expr_type, char* c, char* end_size);

char* generate_operation_asm(Operator_node* n, char* type, char* c) {
    char* l_buf = heap_alloc(100);
    char* l = generate_expression_asm(n->left, type, c, l_buf);
    char* r_buf = heap_alloc(100);
    char* r = generate_expression_asm(n->right, type, c, r_buf);
    char* op_name = heap_alloc(100);
    snprintf(op_name, 100, "%%%d", var_c++);
    strcat(c, op_name);
    strcat(c, " = ");
    strcat(c, find_operation_asm(n->oper, type));
    strcat(c, " ");
    strcat(c, type);
    strcat(c, " ");
    strcat(c, l);
    strcat(c, ", ");
    strcat(c, r);
    strcat(c, "\n");
    return op_name;
}

char* generate_expression_asm(Node* n, char* expr_type, char* c, char* end_size) {
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
        if (!strcmp(type(n), "string")) {
            char* id_name = heap_alloc(100);
            snprintf(id_name, 100, "%s", ht_get(str_tracker, ((Identifier_node*) n)->name));
            return id_name;
        } else {
            char* id_name = heap_alloc(100);
            snprintf(id_name, 100, "%%%s", ((Identifier_node*) n)->name);
            return id_name;
        }
    } else if (n->n_type == REAL_NODE) {
        char number[100];
        snprintf(number, 100, "%f", ((Real_node*) n)->value);
        char* real_name = heap_alloc(100);
        snprintf(real_name, 100, "%%%d", var_c);
        strcat(c, real_name);
        strcat(c, " = fadd double 0.0, ");
        strcat(c, number);
        var_c++;
        strcat(c, "\n");
        return real_name;
    } else if (n->n_type == STRING_NODE) {
        char str[100];
        snprintf(str, 100, "%s", ((String_node*) n)->value);
        char* string_decl = heap_alloc(100);
        char* len = heap_alloc(100);
        snprintf(len, sizeof(len), "%lu", strlen(str) - 2);
        sprintf(string_decl, "@.str.%d = private unnamed_addr constant [%s x i8] c\"%s\"\n", str_c, len, str);
        strcpy(end_size, len);
        insert(c, 0, strlen(c) + 1, string_decl);
        char* str_name = heap_alloc(100);
        snprintf(str_name, 100, "%%%d", var_c++);
        strcat(c, str_name);
        strcat(c, " = alloca i8*, align 8\n");
        strcat(c, "store i8* getelementptr inbounds ([");
        strcat(c, len);
        char* str_name_llvm_form = heap_alloc(100);
        snprintf(str_name_llvm_form, 100, "@.str.%d", str_c);
        ht_set(str_tracker, str_name, str_name_llvm_form);
        strcat(c, " x i8], [");
        strcat(c, len);
        strcat(c, " x i8]* @.str.");
        char* s_c = heap_alloc(100);
        snprintf(s_c, sizeof(s_c), "%d", str_c++);
        strcat(c, s_c);
        strcat(c, ", i32 0, i32 0), i8** ");
        strcat(c, str_name);
        strcat(c, ", align 8\n");
        return str_name;
    }
    
    return generate_operation_asm((Operator_node*) n, expr_type, c);
}

void generate(Node** ast, int size, char* code) {
    str_tracker = ht_create();
    strcat(code, "@.real = private unnamed_addr constant [4 x i8] c\"%f\\0A\\00\"\n@.num = private unnamed_addr constant [4 x i8] c\"%d\\0A\\00\"\n\ndefine i32 @main() {\n");
    heap_init();
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        printf("%d\n", n->n_type);
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char* var_buf = heap_alloc(100);
            char* var_name = generate_expression_asm(v->value, types(v->type), code, var_buf);
            if (!strcmp(v->type, "int")) {
                strcat(code, "%");
                strcat(code, v->name);
                strcat(code, " = add i32 0, ");
                strcat(code, var_name);
            } else if (!strcmp(v->type, "string")) {
                strcat(code, "store i8* ");
                strcat(code, "%");
                strcat(code, v->name);
                strcat(code, ", i8* ");
                strcat(code, var_name);
                strcat(code, ", align 8");
            } else if (!strcmp(v->type, "real")) {
                strcat(code, "%");
                strcat(code, v->name);
                strcat(code, " = load double, double* ");
                strcat(code, var_name);
                strcat(code, ", align 8");
            }
            strcat(code, "\n");
        } else if (n->n_type == WRITE_NODE) {
            Func_call_node* func = (Func_call_node*) n;
            char* end_len = heap_alloc(100);
            char* write_arg_name = generate_expression_asm(func->args[0], types(type(func->args[0])), code, end_len);
            if (!strcmp(type(func->args[0]), "int")) {
                strcat(code, "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.num, i32 0, i32 0), i32 ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            } else if (!strcmp(type(func->args[0]), "string")) {
                strcat(code, "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([");
                strcat(code, end_len);
                strcat(code, " x i8], [");
                strcat(code, end_len);
                strcat(code, " x i8]* ");
                strcat(code, ht_get(str_tracker, write_arg_name));
                strcat(code, ", i32 0, i32 0))");
            } else if (!strcmp(type(func->args[0]), "real")) {
                strcat(code, "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.real, i32 0, i32 0), double ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            }
            strcat(code, "\n");
        }
    }
    strcat(code, "ret i32 0\n}\n\ndeclare i32 @printf(i8* noalias nocapture, ...)\n");
    heap_free_all();
}
