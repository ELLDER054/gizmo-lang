#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "heap.h"
#include "codegen.h"
#include "log.h"
#include "../front-end/ast.h"
#include "../front-end/symbols.h"

#define MAX_BUF_LEN 2048

char* current_function_return_type;
int var_c = 0;
int save_var_c = 0;
int str_c = 1;
char* type(Node* n);
int previous_str_is_ptr = 0;

void enter_function() {
    save_var_c = var_c;
    var_c = 0;
}

void leave_function() {
    var_c = save_var_c;
    save_var_c = 0;
}

char* types(char* t) {
    if (strcmp(t, "int") == 0) {
        return "i32";
    } else if (strcmp(t, "real") == 0) {
        return "double";
    } else if (strcmp(t, "char") == 0) {
        return "i32";
    } else if (strcmp(t, "string") == 0) {
        return "i8*";
    } else if (strcmp(t, "none") == 0) {
        return "void";
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
                return "sdiv";
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

char* generate_operation_asm(Operator_node* n, char* expr_type, char* c) {
    char* l_buf = heap_alloc(100);
    char* l = generate_expression_asm(n->left, expr_type, c, l_buf);
    char* r_buf = heap_alloc(100);
    char* r = generate_expression_asm(n->right, expr_type, c, r_buf);
    char* op_name = heap_alloc(100);
    snprintf(op_name, 100, "%%%d", var_c++);
    strcat(c, "\t");
    strcat(c, op_name);
    strcat(c, " = ");
    char* oper = ((Operator_node*) n)->oper;
    strcat(c, find_operation_asm(oper, expr_type));
    strcat(c, " ");
    strcat(c, expr_type);
    strcat(c, " ");
    strcat(c, l);
    strcat(c, ", ");
    strcat(c, r);
    strcat(c, "\n");
    return op_name;
}

char* generate_expression_asm(Node* n, char* expr_type, char* c, char* end_size) {
    log_trace("type: %d\n", n->n_type);
    if (n->n_type == INTEGER_NODE) {
        char number[100];
        snprintf(number, 100, "%d", ((Integer_node*) n)->value);
        char* int_name = heap_alloc(100);
        snprintf(int_name, 100, "%%%d", var_c++);
        strcat(c, "\t");
        strcat(c, int_name);
        strcat(c, " = add i32 0, ");
        strcat(c, number);
        strcat(c, "\n");
        return int_name;
    } else if (n->n_type == ID_NODE) {
        char* id_name = heap_alloc(105);
        if (strcmp(expr_type, "string") == 0) {
            snprintf(id_name, 100, "%%%d", var_c++);
            char* id_code = heap_alloc(164);
            snprintf(id_code, 164, "\t%s = alloca i8*, align 8\n\tstore i8* %%%s, i8** %s", id_name, ((Identifier_node*) n)->codegen_name, id_name);
            strcat(c, id_code);
            previous_str_is_ptr = 1;
        } else {
            snprintf(id_name, 105, "%%%s", ((Identifier_node*) n)->codegen_name);
        }
        return id_name;
    } else if (n->n_type == CHAR_NODE) {
        char* char_name = heap_alloc(100);
        snprintf(char_name, 100, "%%%d", var_c++);
        char* digit_char = heap_alloc(100);
        snprintf(digit_char, 100, "%d", (int)(((Char_node*) n)->value));
        strcat(c, "\t");
        strcat(c, char_name);
        strcat(c, " = add i32 0, ");
        strcat(c, digit_char);
        strcat(c, "\n");
        return char_name;
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
        strcat(c, "\t");
        strcat(c, str_name);
        strcat(c, " = alloca i8*, align 8\n\tstore i8* getelementptr inbounds ([");
        previous_str_is_ptr = 1;
        strcat(c, len);
        strcat(c, " x i8], [");
        strcat(c, len);
        strcat(c, " x i8]* ");
        strcat(c, str_llvm_name);
        strcat(c, ", i64 0, i64 0), i8** ");
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
        strcat(c, "\t");
        strcat(c, real_name);
        strcat(c, " = fadd double 0.0, ");
        strcat(c, number);
        var_c++;
        strcat(c, "\n");
        return real_name;
    } else if (n->n_type == FUNC_CALL_NODE) {
        char* call = malloc(1024);
        memset(call, 0, 1024);
        char* arg_code = malloc(100);
        memset(arg_code, 0, 100);
        for (int i = 0; i < ((Func_call_node*) n)->args_len; i++) {
            char* arg_buf = heap_alloc(100);
            char* arg = generate_expression_asm(((Func_call_node*) n)->args[i], type(((Func_call_node*) n)->args[i]), c, arg_buf);
            strcat(arg_code, types(type(((Func_call_node*) n)->args[i])));
            strcat(arg_code, " ");
            strcat(arg_code, arg);
            if (i + 1 < ((Func_call_node*) n)->args_len) {
                strcat(arg_code, ", ");
            }
        }
        char* func_call_name = heap_alloc(100);
        snprintf(func_call_name, 100, "%%%d", var_c++);
        snprintf(call, 1024, "\t%s = call %s @%s(%s)\n", func_call_name, types(symtab_find_global(((Func_call_node*) n)->name, "func")->type), ((Func_call_node*) n)->name, arg_code);
        strcat(c, call);
        free(call);
        free(arg_code);
        return func_call_name;
    } else if (n->n_type == READ_NODE) {
        char* func_call_name = heap_alloc(100);
        snprintf(func_call_name, 100, "%%%d", var_c++);
        char* temp_var = heap_alloc(100);
        snprintf(temp_var, 100, "%%%d", var_c++);
        strcat(c, "\t");
        strcat(c, func_call_name);
        strcat(c, " = alloca [1024 x i8], align 8\n\t");
        strcat(c, temp_var);
        strcat(c, " = getelementptr inbounds [1024 x i8], [1024 x i8]* ");
        strcat(c, func_call_name);
        strcat(c, ", i32 0, i32 0\n\t");
        strcat(c, "call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.strnn, i32 0, i32 0), i8* ");
        previous_str_is_ptr = 1;
        strcat(c, temp_var);
        var_c++;
        char* temp_var2 = heap_alloc(100);
        snprintf(temp_var2, 100, "%%%d", var_c++);
        strcat(c, ")\n\t");
        strcat(c, temp_var2);
        strcat(c, " = alloca i8*\n\tstore i8*");
        strcat(c, temp_var);
        strcat(c, ", i8** ");
        strcat(c, temp_var2);
        return temp_var2;
    }
    
    return generate_operation_asm((Operator_node*) n, expr_type, c);
}

void generate_statement(Node* n, char* code) {
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char* var_buf = heap_alloc(100);
            char* var_name = generate_expression_asm(v->value, types(v->type), code, var_buf);
            if (strcmp(v->type, "int") == 0) {
                strcat(code, "\t%");
                strcat(code, v->codegen_name);
                strcat(code, " = add i32 0, ");
                strcat(code, var_name);
            } else if (strcmp(v->type, "string") == 0) {
                if (previous_str_is_ptr) {
                    strcat(code, "\t%");
                    strcat(code, v->codegen_name);
                    strcat(code, " = load i8*, i8*");
                    strcat(code, "* ");
                    previous_str_is_ptr = 0;
                    strcat(code, var_name);
                    strcat(code, ", align 8\n");
                } else {
                    strcat(code, "\n\tstore i8*");
                    strcat(code, var_name);
                    strcat(code, ", i8* %");
                    strcat(code, v->codegen_name);
                    strcat(code, "\n");
                }
            } else if (strcmp(v->type, "char") == 0) {
                strcat(code, "\t%");
                strcat(code, v->codegen_name);
                strcat(code, " = add i32 0, ");
                strcat(code, var_name);
            } else if (strcmp(v->type, "real") == 0) {
                strcat(code, "\t%");
                strcat(code, v->codegen_name);
                strcat(code, " = fadd double 0.0, ");
                strcat(code, var_name);
            }
            strcat(code, "\n");
        } else if (n->n_type == WRITE_NODE) {
            Func_call_node* func = (Func_call_node*) n;
            char* end_len = heap_alloc(100);
            char* write_arg_name = generate_expression_asm(func->args[0], types(type(func->args[0])), code, end_len);
            if (strcmp(type(func->args[0]), "int") == 0) {
                strcat(code, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.num, i32 0, i32 0), i32 ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            } else if (strcmp(type(func->args[0]), "string") == 0) {
                char* name = heap_alloc(100);
                if (previous_str_is_ptr) {
                    snprintf(name, 100, "%%%d", var_c++);
                    strcat(code, "\t");
                    strcat(code, name);
                    strcat(code, " = load i8*, i8** ");
                    strcat(code, write_arg_name);
                    strcat(code, ", align 8\n");
                    strcat(code, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i8* ");
                    strcat(code, name);
                    strcat(code, ")");
                } else {
                    strcat(code, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i8* ");
                    strcat(code, write_arg_name);
                    strcat(code, ")");
                }
            } else if (strcmp(type(func->args[0]), "char") == 0) {
                strcat(code, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.chr, i32 0, i32 0), i32 ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            } else if (strcmp(type(func->args[0]), "real") == 0) {
                strcat(code, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.real, i32 0, i32 0), double ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            }
            strcat(code, "\n");
            var_c++;
        } else if (n->n_type == READ_NODE) {
            strcat(code, "\n");
        } else if (n->n_type == BLOCK_NODE) {
            for (int i = 0; i < ((Block_node*) n)->ssize; i++) {
                generate_statement(((Block_node*) n)->statements[i], code);
            }
        } else if (n->n_type == FUNC_CALL_NODE) {
            char* call = malloc(195);
            memset(call, 0, 195);
            char* arg_code = malloc(100);
            memset(arg_code, 0, 100);
            for (int i = 0; i < ((Func_call_node*) n)->args_len; i++) {
                char* arg_buf = heap_alloc(100);
                char* arg = generate_expression_asm(((Func_call_node*) n)->args[i], type(((Func_call_node*) n)->args[i]), code, arg_buf);
                strcat(arg_code, types(type(((Func_call_node*) n)->args[i])));
                strcat(arg_code, " ");
                strcat(arg_code, arg);
                if (i + 1 < ((Func_call_node*) n)->args_len) {
                    strcat(arg_code, ", ");
                }
            }
            snprintf(call, 195, "call %s @%s(%s)\n", types(symtab_find_global(((Func_call_node*) n)->name, "func")->type), ((Func_call_node*) n)->name, arg_code);
            strcat(code, call);
            free(call);
            free(arg_code);
        } else if (n->n_type == FUNC_DECL_NODE) {
            char* mini_code = malloc(1024);
            memset(mini_code, 0, 1024);
            char* begin = malloc(195);
            memset(begin, 0, 195);
            char* arg_code = malloc(100);
            memset(arg_code, 0, 100);
            for (int i = 0; i < ((Func_decl_node*) n)->args_len; i++) {
                Var_declaration_node* arg = (Var_declaration_node*) ((Func_decl_node*) n)->args[i];
                strcat(arg_code, types(arg->type));
                strcat(arg_code, " %");
                strcat(arg_code, arg->codegen_name);
                if (i + 1 < ((Func_decl_node*) n)->args_len) {
                    strcat(arg_code, ", ");
                }
            }
            log_trace("funtion type %s\n", types(((Func_decl_node*) n)->type));
            snprintf(begin, 195, "define %s @%s(%s) {\nentry:\n", types(((Func_decl_node*) n)->type), ((Func_decl_node*) n)->name, arg_code);
            strcat(mini_code, begin);
            strcpy(current_function_return_type, ((Func_decl_node*) n)->type);
            enter_function();
            generate_statement(((Func_decl_node*) n)->body, mini_code);
            leave_function();
            strcpy(current_function_return_type, "");
            if (strcmp(types(((Func_decl_node*) n)->type), "void") == 0) {
                strcat(mini_code, "\tret void\n");
            }
            strcat(mini_code, "}\n");
            insert(code, 0, strlen(code), mini_code);
            free(mini_code);
            free(begin);
            free(arg_code);
        } else if (n->n_type == RET_NODE) {
            char* end_len = malloc(100);
            char* ret_name = generate_expression_asm(((Return_node*) n)->expr, types(current_function_return_type), code, end_len);
            strcat(code, "\tret ");
            strcat(code, types(current_function_return_type));
            strcat(code, " ");
            strcat(code, ret_name);
            strcat(code, "\n");
        } else {
            fprintf(stderr, "gizmo: This feature (%d) is either not yet implemented in the back-end or there is an internal compiler error\nPlease report this error, along with the number in the parenthesis, to the developers at gizmo@gizmolang.org\n", n->n_type);
            exit(-1);
        }
}

void generate(Node** ast, int size, char* code, char* file_name) {
    current_function_return_type = malloc(100);
    memset(current_function_return_type, 0, 100);
    strcpy(code, "@.chr = private unnamed_addr constant [4 x i8] c\"%c\\0A\\00\"\n@.strnn = private unnamed_addr constant [7 x i8] c \"%1024s\\00\"\n@.str = private unnamed_addr constant [4 x i8] c\"%s\\0A\\00\"\n@.real = private unnamed_addr constant [4 x i8] c\"%f\\0A\\00\"\n@.num = private unnamed_addr constant [4 x i8] c\"%d\\0A\\00\"\n\ndefine double @div_int(i32 %a, i32 %b) {\nentry:\n\t%0 = sitofp i32 %a to double\n\t%1 = sitofp i32 %b to double\n\t%2 = fdiv double %0, %1\n\tret double %2\n}\n\ndefine i32 @main() {\nentry:\n");
    heap_init();
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        generate_statement(n, code);
    }
    char* module_id = heap_alloc(400);
    snprintf(module_id, 400, "; ModuleID = '%s'\nsource_filename = \"%s\"\n", file_name, file_name);
    insert(code, 0, strlen(code), module_id);
    strcat(code, "\tret i32 0\n}\n\ndeclare i32 @scanf(i8*, ...)\ndeclare i32 @printf(i8* noalias nocapture, ...)\n");
    heap_free_all();
    symtab_destroy();
}
