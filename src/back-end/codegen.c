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
int previous_str_is_ptr = 0;

char* types(char* t) {
    if (strcmp(t, "int") == 0) {
        return "i32";
    } else if (strcmp(t, "real") == 0) {
        return "double";
    }
    return "";
}

void insert(char* buf, int pos, int size, char* str) {
    char* temp = heap_alloc(MAX_BUF_LEN * 2);
    for (int i = 0; i < pos; i++) {
        char c = buf[i];
        strncat(temp, &c, 1);
    }
    strcat(temp, str);
    for (int i = pos; i < size; i++) {
        char c = buf[i];
        strncat(temp, &c, 1);
    }
    strcpy(buf, temp);
}

char* find_operation_asm(char* oper, char* t) {
    if (strcmp(t, "i32") == 0) {
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
    } else if (strcmp(t, "double") == 0) {
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
        char* id_name = heap_alloc(100);
        if (strcmp(expr_type, "string") == 0) {
            snprintf(id_name, 100, "%%%d", var_c++);
            char* id_code = heap_alloc(100);
            snprintf(id_code, 100, "%s = alloca i8*, align 8\nstore i8* %%%s, i8** %s", id_name, ((Identifier_node*) n)->name, id_name);
            strcat(c, id_code);
            previous_str_is_ptr = 1;
        } else {
            snprintf(id_name, 100, "%%%s", ((Identifier_node*) n)->name);
        }
        return id_name;
    } else if (n->n_type == STRING_NODE) {
        char str[100];
        snprintf(str, 100, "%s", ((String_node*) n)->value);
        char str_llvm_name[106];
        snprintf(str_llvm_name, 100, "@.str.%d", str_c++);
        char* str_name = heap_alloc(100);
        snprintf(str_name, 100, "%%%d", var_c);
        char* str_assignment = heap_alloc(100);
        snprintf(str_assignment, 400, "%s = private unnamed_addr constant [%lu x i8] c\"%s\"\n", str_llvm_name, strlen(str) - 2, str);
        insert(c, 0, strlen(c), str_assignment);
        char* len = heap_alloc(100);
        snprintf(len, 100, "%lu", strlen(str) - 2);
        strcat(c, str_name);
        strcat(c, " = alloca i8*, align 8\nstore i8* getelementptr inbounds ([");
        previous_str_is_ptr = 1;
        strcat(c, len);
        strcat(c, " x i8], [");
        strcat(c, len);
        strcat(c, " x i8]* @.str.1, i64 0, i64 0), i8** ");
        strcat(c, str_name);
        strcat(c, ", align 8\n");
        var_c++;
        strcat(c, "\n");
        return str_name;
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
    }
    
    return generate_operation_asm((Operator_node*) n, expr_type, c);
}

void generate(Node** ast, int size, char* code, char* file_name) {
    strcat(code, "@.str = private unnamed_addr constant [4 x i8] c\"%s\\0A\\00\"\n@.real = private unnamed_addr constant [4 x i8] c\"%f\\0A\\00\"\n@.num = private unnamed_addr constant [4 x i8] c\"%d\\0A\\00\"\n\ndefine i32 @main() {\n");
    heap_init();
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char* var_buf = heap_alloc(100);
            char* var_name = generate_expression_asm(v->value, types(v->type), code, var_buf);
            if (strcmp(v->type, "int") == 0) {
                strcat(code, "%");
                strcat(code, v->name);
                strcat(code, " = add i32 0, ");
                strcat(code, var_name);
            } else if (strcmp(v->type, "string") == 0) {
                strcat(code, "%");
                strcat(code, v->name);
                strcat(code, " = load i8*, i8*");
                if (previous_str_is_ptr) {
                    strcat(code, "* ");
                    previous_str_is_ptr = 0;
                } else {
                    strcat(code, " ");
                }
                strcat(code, var_name);
                strcat(code, ", align 8\n");
            } else if (strcmp(v->type, "real") == 0) {
                strcat(code, "%");
                strcat(code, v->name);
                strcat(code, " = fadd double 0.0, ");
                strcat(code, var_name);
            }
            strcat(code, "\n");
        } else if (n->n_type == WRITE_NODE) {
            Func_call_node* func = (Func_call_node*) n;
            char* end_len = heap_alloc(100);
            char* write_arg_name = generate_expression_asm(func->args[0], types(type(func->args[0])), code, end_len);
            if (strcmp(type(func->args[0]), "int") == 0) {
                strcat(code, "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.num, i32 0, i32 0), i32 ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            } else if (strcmp(type(func->args[0]), "string") == 0) {
                char* name = heap_alloc(100);
                if (previous_str_is_ptr) {
                    snprintf(name, 100, "%%%d", var_c++);
                    strcat(code, name);
                    strcat(code, " = load i8*, i8** ");
                    strcat(code, write_arg_name);
                    strcat(code, ", align 8\n");
                    strcat(code, "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i8* ");
                    strcat(code, name);
                    strcat(code, ")");
                } else {
                    strcat(code, "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i8* ");
                    strcat(code, write_arg_name);
                    strcat(code, ")");
                }
            } else if (strcmp(type(func->args[0]), "real") == 0) {
                strcat(code, "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.real, i32 0, i32 0), double ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            }
            strcat(code, "\n");
        }
    }
    char* module_id = heap_alloc(100);
    snprintf(module_id, 400, "; ModuleID = '%s'\n", file_name);
    insert(code, 0, strlen(code), module_id);
    strcat(code, "ret i32 0\n}\n\ndeclare i32 @printf(i8* noalias nocapture, ...)\n");
    heap_free_all();
}
