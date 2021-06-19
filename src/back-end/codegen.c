#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "heap.h"
#include "codegen.h"
#include "../common/include/log.h"
#include "../front-end/ast.h"
#include "../front-end/symbols.h"

#define MAX_BUF_LEN 2048

/*
define i8* @constr(i8* %a, i8* %b) {
entry:
    %0 = call i8* @malloc(i32 6)
    call i8* @strcpy(i8* %0, i8* %a)
    call i8* @strcat(i8* %0, i8* %b)
    ret i8* %0
}
*/

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
        return "i8";
    } else if (strcmp(t, "string") == 0) {
        return "i8*";
    } else if (strcmp(t, "bool") == 0) {
        return "i1";
    }
    return "";
}

void insert(char* buf, int pos, int size, char* str) {
    char* temp = malloc(MAX_BUF_LEN * 2);
    memset(temp, 0, MAX_BUF_LEN * 2);
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
    free(temp);
}

int needs_constr = 0;
int needs_cmpstr = 0;
int needs_strcmp = 0;
int needs_div_int = 0;
int needs_str_const = 0;
int needs_true_const = 0;
int needs_false_const = 0;
int needs_num_const = 0;
int needs_real_const = 0;
int needs_chr_const = 0;
int needs_strnn_const = 0;
int needs_printf = 0;
int needs_scanf = 0;
int needs_malloc = 0;
int needs_strcpy = 0;
int needs_strcat = 0;
int needs_strlen = 0;

int bool_c = 0;
int end_c = 0;
int while_c = 0;
int if_c = 0;
int else_c = 0;

char* find_operation_asm(char* oper, char* t) {
    if (strcmp(t, "i32") == 0 || strcmp(t, "i8") == 0) {
        if (strcmp(oper, "+") == 0) {
            return "add";
        } else if (strcmp(oper, "-") == 0) {
            return "sub";
        } else if (strcmp(oper, "*") == 0) {
            return "mul";
        } else if (strcmp(oper, "/") == 0) {
            return "sdiv";
        } else if (strcmp(oper, "%") == 0) {
            return "srem";
        } else if (strcmp(oper, ">") == 0) {
            return "icmp sgt";
        } else if (strcmp(oper, "<") == 0) {
            return "icmp slt";
        } else if (strcmp(oper, ">=") == 0) {
            return "icmp sge";
        } else if (strcmp(oper, "<=") == 0) {
            return "icmp sle";
        } else if (strcmp(oper, "==") == 0) {
            return "icmp eq";
        } else if (strcmp(oper, "!=") == 0) {
            return "icmp ne";
        }
    } else if (strcmp(t, "double") == 0) {
        if (strcmp(oper, "+") == 0) {
            return "fadd";
        } else if (strcmp(oper, "-") == 0) {
            return "fsub";
        } else if (strcmp(oper, "*") == 0) {
            return "fmul";
        } else if (strcmp(oper, "/") == 0) {
            return "fdiv";
        } else if (strcmp(oper, ">") == 0) {
            return "fcmp ogt";
        } else if (strcmp(oper, "<") == 0) {
            return "fcmp olt";
        } else if (strcmp(oper, ">=") == 0) {
            return "fcmp oge";
        } else if (strcmp(oper, "<=") == 0) {
            return "fcmp ole";
        } else if (strcmp(oper, "==") == 0) {
            return "fcmp oeq";
        } else if (strcmp(oper, "!=") == 0) {
            return "fcmp one";
        }
    } else if (strcmp(t, "i8*") == 0)  {
        if (strcmp(oper, "+") == 0) {
            needs_constr = 1;
            return "call i8* @constr(";
        } else if (strcmp(oper, "==") == 0) {
            needs_cmpstr = 1;
            return "call i1 @cmpstr(";
        } else if (strcmp(oper, "!=") == 0) {
            needs_strcmp = 1;
            return "call i1 @strcmp(";
        }
    } else if (strcmp(t, "i1") == 0) {
        if (strcmp(oper, "and") == 0) {
            return "and";
        } else if (strcmp(oper, "or") == 0) {
            return "or";
        } else if (strcmp(oper, "==") == 0) {
            return "icmp eq";
        } else if (strcmp(oper, "!=") == 0) {
            return "icmp ne";
        }
    } else if (strcmp(t, "i8") == 0) {
        if (strcmp(oper, "+") == 0) {
            return "add";
        } else if (strcmp(oper, "-") == 0) {
            return "sub";
        } else if (strcmp(oper, "*") == 0) {
            return "mul";
        } else if (strcmp(oper, "/") == 0) {
            return "sdiv";
        } else if (strcmp(oper, ">") == 0) {
            return "icmp sgt";
        } else if (strcmp(oper, "<") == 0) {
            return "icmp slt";
        } else if (strcmp(oper, ">=") == 0) {
            return "icmp sge";
        } else if (strcmp(oper, "<=") == 0) {
            return "icmp sle";
        } else if (strcmp(oper, "==") == 0) {
            return "icmp eq";
        } else if (strcmp(oper, "!=") == 0) {
            return "icmp ne";
        }
    }
    return "";
}

char* generate_expression_asm(Node* n, char* expr_type, char* c, char* end_size);

char* generate_operation_asm(Node* n, char* expr_type, char* c) {
    char* l_buf = malloc(100);
    memset(l_buf, 0, 100);
    char* l = generate_expression_asm(((Operator_node*) n)->left, expr_type, c, l_buf);
    char* r_buf = malloc(100);
    memset(r_buf, 0, 100);
    char* r = generate_expression_asm(((Operator_node*) n)->right, expr_type, c, r_buf);
    free(l_buf);
    free(r_buf);
    char* op_name = malloc(100);
    memset(op_name, 0, 100);
    snprintf(op_name, 100, "%%%d", var_c++);
    strcat(c, "\t");
    strcat(c, op_name);
    strcat(c, " = ");
    char* oper = ((Operator_node*) n)->oper;
    char* oper_asm = find_operation_asm(oper, types(type(((Operator_node*) n)->left)));
    strcat(c, oper_asm);
    strcat(c, " ");
    strcat(c, types(type(((Operator_node*) n)->left)));
    strcat(c, " ");
    strcat(c, l);
    strcat(c, ", ");
    if (oper_asm[strlen(oper_asm) - 1] == '(') {
        strcat(c, types(type(((Operator_node*) n)->left)));
        strcat(c, " ");
    }
    strcat(c, r);
    if (strcmp(oper_asm, "call i8* @constr(") == 0 || strcmp(oper_asm, "call i1 @cmpstr(") == 0 || strcmp(oper_asm, "call i1 @strcmp(") == 0) {
        strcat(c, ")");
    }
    /*if (strlen(oper_asm) > 0 && oper_asm[1] == 'c') {
        char* op_name_new = heap_alloc(100);
        snprintf(op_name_new, 100, "%%%d", var_c++);
        strcat(c, "\n\t");
        strcat(c, op_name_new);
        strcat(c, " = zext i1 ");
        strcat(c, op_name);
        strcat(c, " to i32\n");
        return op_name_new;
    }*/
    strcat(c, "\n");
    return op_name;
}

int gizmo_strlen(char* str) {
    int pos = 0;
    int len = 0;
    while (pos < strlen(str)) {
        char c = str[pos];
        if (c == '\\') {
            len++;
            pos += 3;
        } else {
            len++;
            pos++;
        }
    }
    return len;
}

char* generate_expression_asm(Node* n, char* expr_type, char* c, char* end_size) {
    log_trace("type: %d\n", n->n_type);
    if (n->n_type == INTEGER_NODE) {
        char* number = heap_alloc(100);
        snprintf(number, 100, "%d", ((Integer_node*) n)->value);
        return number;
    } else if (n->n_type == ID_NODE) {
        char* id_name = heap_alloc(105);
        char* id_code = malloc(164);
        memset(id_code, 0, 164);
        char* id_type = ((Identifier_node*) n)->type;
        if (strcmp(id_type, "string") == 0 || strcmp(id_type, "i8*") == 0) {
            snprintf(id_name, 100, "%%%d", var_c++);
            snprintf(id_code, 164, "\t%s = load i8*, i8** %%%s\n", id_name, ((Identifier_node*) n)->codegen_name);
            strcat(c, id_code);
            previous_str_is_ptr = 1;
        } else if (strcmp(id_type, "int") == 0 || strcmp(id_type, "i32") == 0) {
            snprintf(id_name, 100, "%%%d", var_c++);
            snprintf(id_code, 164, "\t%s = load i32, i32* %%%s\n", id_name, ((Identifier_node*) n)->codegen_name);
            strcat(c, id_code);
            previous_str_is_ptr = 1;
        } else if (strcmp(id_type, "real") == 0 || strcmp(id_type, "double") == 0) {
            snprintf(id_name, 100, "%%%d", var_c++);
            snprintf(id_code, 164, "\t%s = load double, double* %%%s\n", id_name, ((Identifier_node*) n)->codegen_name);
            strcat(c, id_code);
            previous_str_is_ptr = 1;
        } else if (strcmp(id_type, "char") == 0 || strcmp(id_type, "i8") == 0) {
            snprintf(id_name, 100, "%%%d", var_c++);
            snprintf(id_code, 164, "\t%s = load i8, i8* %%%s\n", id_name, ((Identifier_node*) n)->codegen_name);
            strcat(c, id_code);
            previous_str_is_ptr = 1;
        } else if (strcmp(id_type, "bool") == 0 || strcmp(id_type, "i1") == 0) {
            snprintf(id_name, 100, "%%%d", var_c++);
            char* id_code = heap_alloc(164);
            snprintf(id_code, 164, "\t%s = load i1, i1* %%%s\n", id_name, ((Identifier_node*) n)->codegen_name);
            strcat(c, id_code);
            previous_str_is_ptr = 1;
        } else {
            snprintf(id_name, 105, "%%%s", ((Identifier_node*) n)->codegen_name);
        }
        free(id_code);
        return id_name;
    } else if (n->n_type == CHAR_NODE) {
        char* digit_char = heap_alloc(100);
        snprintf(digit_char, 100, "%d", (int)(((Char_node*) n)->value));
        return digit_char;
    } else if (n->n_type == STRING_NODE) {
        char str[100];
        snprintf(str, 100, "%s", ((String_node*) n)->value);
        char str_llvm_name[106];
        snprintf(str_llvm_name, 100, "@.str.%d", str_c++);
        char* str_name = malloc(100);
        memset(str_name, 0, 100);
        snprintf(str_name, 100, "%%%d", var_c++);
        char* str_assignment = malloc(100);
        memset(str_assignment, 0, 100);
        snprintf(str_assignment, 400, "%s = private unnamed_addr constant [%d x i8] c\"%s\"\n", str_llvm_name, gizmo_strlen(str), str);
        insert(c, 0, strlen(c), str_assignment);
        free(str_assignment);
        char* len = malloc(100);
        memset(len, 0, 100);
        snprintf(len, 100, "%d", gizmo_strlen(str));
        strcat(c, "\t");
        strcat(c, str_name);
        strcat(c, " = getelementptr inbounds [");
        previous_str_is_ptr = 1;
        strcat(c, len);
        strcat(c, " x i8], [");
        strcat(c, len);
        strcat(c, " x i8]* ");
        strcat(c, str_llvm_name);
        strcat(c, "\n\t");
        char* extra_name = heap_alloc(100);
        snprintf(extra_name, 100, "%%%d", var_c++);
        strcat(c, extra_name);
        strcat(c, " = bitcast [");
        strcat(c, len);
        free(len);
        strcat(c, " x i8]* ");
        strcat(c, str_name);
        free(str_name);
        strcat(c, " to i8*\n");
        return extra_name;
    } else if (n->n_type == REAL_NODE) {
        char* number = heap_alloc(100);
        snprintf(number, 100, "%f", ((Real_node*) n)->value);
        return number;
    } else if (n->n_type == BOOL_NODE) {
        char* number = heap_alloc(100);
        snprintf(number, 100, "%d", ((Boolean_node*) n)->value);
        return number;
    } else if (n->n_type == FUNC_CALL_NODE) {
        char* call = malloc(1024);
        memset(call, 0, 1024);
        char* arg_code = malloc(100);
        memset(arg_code, 0, 100);
        for (int i = 0; i < ((Func_call_node*) n)->args_len; i++) {
            char* arg_buf = malloc(100);
            memset(arg_buf, 0, 100);
            char* arg = generate_expression_asm(((Func_call_node*) n)->args[i], type(((Func_call_node*) n)->args[i]), c, arg_buf);
            strcat(c, "\t");
            char* extra_name = malloc(100);
            memset(extra_name, 0, 100);
            snprintf(extra_name, 100, "%%%d", var_c++);
            strcat(c, extra_name);
            strcat(c, " = alloca i32\n\tstore i32 ");
            strcat(c, arg);
            strcat(c, ", i32* ");
            strcat(c, extra_name);
            strcat(c, "\n");
            strcat(arg_code, types(type(((Func_call_node*) n)->args[i])));
            strcat(arg_code, "* ");
            strcat(arg_code, extra_name);
            free(extra_name);
            if (i + 1 < ((Func_call_node*) n)->args_len) {
                strcat(arg_code, ", ");
            }
            free(arg_buf);
        }
        char* func_call_name = heap_alloc(100);
        snprintf(func_call_name, 100, "%%%d", var_c++);
        snprintf(call, 1024, "\t%s = call %s @%s(%s)\n", func_call_name, types(symtab_find_global(((Func_call_node*) n)->name, "func")->type), ((Func_call_node*) n)->name, arg_code);
        strcat(c, call);
        free(call);
        free(arg_code);
        return func_call_name;
    } else if (n->n_type == READ_NODE) {
        needs_scanf = 1;
        if (strcmp(type(((Func_call_node*) n)->args[0]), "string")) {
            fprintf(stderr, "error: Argument 1 of the 'read' function must be a string\n");
        }
        needs_strnn_const = 1;
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
        char end_len[100];
        char* write_name = generate_expression_asm(((Func_call_node*) n)->args[0], "string", c, end_len);
        strcat(c, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.strnn, i32 0, i32 0), i8* ");
        strcat(c, write_name);
        var_c++;
        strcat(c, ")\n\t");
        strcat(c, "call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.strnn, i32 0, i32 0), i8* ");
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
        strcat(c, "\n");
        char* last_temp_var = heap_alloc(100);
        snprintf(last_temp_var, 100, "%%%d", var_c++);
        strcat(c, last_temp_var);
        strcat(c, " = load i8*, i8** ");
        strcat(c, temp_var2);
        strcat(c, "\n");
        return last_temp_var;
    }
    
    return generate_operation_asm(n, expr_type, c);
}

void generate_statement(Node* n, char* code) {
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char* var_buf = heap_alloc(100);
            char* var_name = generate_expression_asm(v->value, v->type, code, var_buf);
            if (strcmp(v->type, "int") == 0) {
                strcat(code, "\t%");
                strcat(code, v->codegen_name);
                strcat(code, " = alloca i32\n\tstore i32 ");
                strcat(code, var_name);
                strcat(code, ", i32* %");
                strcat(code, v->codegen_name);
            } else if (strcmp(v->type, "string") == 0) {
                strcat(code, "\t%");
                strcat(code, v->codegen_name);
                strcat(code, " = alloca i8*\n\tstore i8* ");
                strcat(code, var_name);
                strcat(code, ", i8** %");
                strcat(code, v->codegen_name);
            } else if (strcmp(v->type, "bool") == 0) {
                strcat(code, "\t%");
                strcat(code, v->codegen_name);
                strcat(code, " = alloca i1\n\tstore i1 ");
                strcat(code, var_name);
                strcat(code, ", i1* %");
                strcat(code, v->codegen_name);
            } else if (strcmp(v->type, "char") == 0) {
                strcat(code, "\t%");
                strcat(code, v->codegen_name);
                strcat(code, " = alloca i8\n\tstore i8 ");
                strcat(code, var_name);
                strcat(code, ", i8* %");
                strcat(code, v->codegen_name);
            } else if (strcmp(v->type, "real") == 0) {
                strcat(code, "\t%");
                strcat(code, v->codegen_name);
                strcat(code, " = alloca double\n\tstore double ");
                strcat(code, var_name);
                strcat(code, ", double* %");
                strcat(code, v->codegen_name);
            }
            strcat(code, "\n");
        } else if (n->n_type == VAR_ASSIGN_NODE) {
            Var_assignment_node* v = (Var_assignment_node*) n;
            char* var_buf = heap_alloc(100);
            char* var_name = generate_expression_asm(v->value, type(v->value), code, var_buf);
            char* new_id = heap_alloc(100);
            if (strcmp(type(v->value), "int") == 0) {
                strcat(code, "\tstore i32 ");
                strcat(code, var_name);
                strcat(code, ", i32* %");
                strcat(code, v->codegen_name);
                strcat(code, "\n");
            } else if (strcmp(type(v->value), "string") == 0) {
                strcat(code, "\tstore i8* ");
                strcat(code, var_name);
                strcat(code, ", i8** %");
                strcat(code, v->codegen_name);
                strcat(code, "\n");
            } else if (strcmp(type(v->value), "bool") == 0) {
                strcat(code, "\tstore i1 ");
                strcat(code, var_name);
                strcat(code, ", i1* %");
                strcat(code, v->codegen_name);
                strcat(code, "\n");
            } else if (strcmp(type(v->value), "char") == 0) {
                strcat(code, "\tstore i8 ");
                strcat(code, var_name);
                strcat(code, ", i8* %");
                strcat(code, v->codegen_name);
                strcat(code, "\n");
            } else if (strcmp(type(v->value), "real") == 0) {
                strcat(code, "\tstore double ");
                strcat(code, var_name);
                strcat(code, ", double* %");
                strcat(code, v->codegen_name);
                strcat(code, "\n");
            }
            strcat(code, "\n");
        } else if (n->n_type == WHILE_NODE) {
            char* end_len = malloc(100);
            memset(end_len, 0, 100);
            While_loop_node* wh = (While_loop_node*) n;
            char* while_name = generate_expression_asm(wh->condition, type(wh->condition), code, end_len);
            strcat(code, "\tbr i1 ");
            strcat(code, while_name);
            strcat(code, ", label %");
            strcat(code, wh->begin_cgid);
            strcat(code, ", label %");
            strcat(code, wh->end_cgid);
            strcat(code, "\n");
            strcat(code, wh->begin_cgid);
            strcat(code, ":\n");
            generate_statement(wh->body, code);
            char* while_name2 = generate_expression_asm(wh->condition, type(wh->condition), code, end_len);
            free(end_len);
            strcat(code, "\tbr i1 ");
            strcat(code, while_name2);
            strcat(code, ", label %");
            strcat(code, wh->begin_cgid);
            strcat(code, ", label %");
            strcat(code, wh->end_cgid);
            strcat(code, "\n");
            strcat(code, wh->end_cgid);
            strcat(code, ":\n");
        } else if (n->n_type == IF_NODE) {
            char* end_len = malloc(100);
            memset(end_len, 0, 100);
            If_node* i = (If_node*) n;
            char* if_name = generate_expression_asm(i->condition, type(i->condition), code, end_len);
            strcat(code, "\tbr i1 ");
            strcat(code, if_name);
            strcat(code, ", label %");
            strcat(code, i->begin_cgid);
            strcat(code, ", label %");
            strcat(code, i->else_body == NULL ? i->end_cgid : i->else_cgid);
            strcat(code, "\n");
            strcat(code, i->begin_cgid);
            strcat(code, ":\n");
            generate_statement(i->body, code);
            free(end_len);
            strcat(code, "\tbr label %");
            strcat(code, i->end_cgid);
            if (i->else_body != NULL) {
                strcat(code, "\n");
                strcat(code, i->else_cgid);
                strcat(code, ":\n");
                generate_statement(i->else_body, code);
                strcat(code, "\tbr label %");
                strcat(code, i->end_cgid);
                strcat(code, "\n");
            }
            strcat(code, "\n");
            strcat(code, i->end_cgid);
            strcat(code, ":\n");
        } else if (n->n_type == WRITE_NODE) {
            Func_call_node* func = (Func_call_node*) n;
            char* end_len = malloc(100);
            memset(end_len, 0, 100);
            needs_printf = 1;
            char* write_arg_name = generate_expression_asm(func->args[0], type(func->args[0]), code, end_len);
            free(end_len);
            if (strcmp(type(func->args[0]), "int") == 0) {
                needs_num_const = 1;
                strcat(code, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.num, i32 0, i32 0), i32 ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            } else if (strcmp(type(func->args[0]), "string") == 0) {
                needs_str_const = 1;
                strcat(code, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i8* ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            } else if (strcmp(type(func->args[0]), "bool") == 0) {
                needs_true_const = 1;
                needs_false_const = 1;
                char* t = malloc(100);
                memset(t, 0, 100);
                snprintf(t, 100, "true%d", bool_c);
                char* f = malloc(100);
                memset(f, 0, 100);
                snprintf(f, 100, "false%d", bool_c);
                char* end = malloc(100);
                memset(end, 0, 100);
                snprintf(end, 100, "end%d", end_c++);
                var_c++;
                strcat(code, "\tbr i1 ");
                strcat(code, write_arg_name);
                strcat(code, ", label %");
                strcat(code, t);
                strcat(code, ", label %");
                strcat(code, f);
                strcat(code, "\n");
                strcat(code, t);
                strcat(code, ":\n\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.true, i32 0, i32 0))\n\tbr label %");
                strcat(code, end);
                strcat(code, "\n");
                strcat(code, f);
                strcat(code, ":\n\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.false, i32 0, i32 0))\n\tbr label %");
                strcat(code, end);
                strcat(code, "\n");
                strcat(code, end);
                strcat(code, ":");
                free(t);
                free(f);
                free(end);
            } else if (strcmp(type(func->args[0]), "char") == 0) {
                needs_chr_const = 1;
                strcat(code, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.chr, i32 0, i32 0), i8 ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            } else if (strcmp(type(func->args[0]), "real") == 0) {
                needs_real_const = 1;
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
                char* arg_buf = malloc(100);
                memset(arg_buf, 0, 100);
                char* arg = generate_expression_asm(((Func_call_node*) n)->args[i], type(((Func_call_node*) n)->args[i]), code, arg_buf);
                strcat(code, "\t");
                char* extra_name = malloc(100);
                memset(extra_name, 0, 100);
                snprintf(extra_name, 100, "%%%d", var_c++);
                strcat(code, extra_name);
                strcat(code, " = alloca i32\n\tstore i32 ");
                strcat(code, arg);
                strcat(code, ", i32* ");
                strcat(code, extra_name);
                strcat(code, "\n");
                strcat(arg_code, types(type(((Func_call_node*) n)->args[i])));
                strcat(arg_code, "* ");
                strcat(arg_code, extra_name);
                free(extra_name);
                if (i + 1 < ((Func_call_node*) n)->args_len) {
                    strcat(arg_code, ", ");
                }
                free(arg_buf);
            }
            snprintf(call, 195, "\tcall %s @%s(%s)\n", types(symtab_find_global(((Func_call_node*) n)->name, "func")->type), ((Func_call_node*) n)->name, arg_code);
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
                strcat(arg_code, "* %");
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
            strcat(mini_code, "}\n");
            insert(code, 0, strlen(code), mini_code);
            free(mini_code);
            free(begin);
            free(arg_code);
        } else if (n->n_type == RET_NODE) {
            char* end_len = malloc(100);
            char* ret_name = generate_expression_asm(((Return_node*) n)->expr, current_function_return_type, code, end_len);
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
    strcat(code, "\ndefine i32 @main() {\nentry:\n");
    heap_init();
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        generate_statement(n, code);
    }
    if (needs_strnn_const) {
        insert(code, 0, strlen(code), "@.strnn = private unnamed_addr constant [3 x i8] c\"%s\\00\"\n");
    }
    if (needs_chr_const) {
        insert(code, 0, strlen(code), "@.chr = private unnamed_addr constant [4 x i8] c\"%c\\0A\\00\"\n");
    }
    if (needs_str_const) {
        insert(code, 0, strlen(code), "@.str = private unnamed_addr constant [4 x i8] c\"%s\\0A\\00\"\n");
    }
    if (needs_real_const) {
        insert(code, 0, strlen(code), "@.real = private unnamed_addr constant [4 x i8] c\"%f\\0A\\00\"\n");
    }
    if (needs_num_const) {
        insert(code, 0, strlen(code), "@.num = private unnamed_addr constant [4 x i8] c\"%d\\0A\\00\"\n");
    }
    if (needs_true_const) {
        insert(code, 0, strlen(code), "@.true = private unnamed_addr constant [6 x i8] c\"true\\0A\\00\"");
    }
    if (needs_false_const) {
        insert(code, 0, strlen(code), "@.false = private unnamed_addr constant [7 x i8] c\"false\\0A\\00\"");
    }
    if (needs_constr) {
        needs_malloc = 1;
        needs_strlen = 1;
        needs_strcpy = 1;
        needs_strcat = 1;
        insert(code, 0, strlen(code), "\ndefine i8* @constr(i8* %a, i8* %b) {\nentry:\n\t%0 = call i32 @strlen(i8* %a)\n\t%1 = call i32 @strlen(i8* %b)\n\t%2 = add i32 %0, %1\n\t%3 = call i8* @malloc(i32 %2)\n\tcall i8* @strcpy(i8* %3, i8* %a)\n\tcall i8* @strcat(i8* %3, i8* %b)\n\tret i8* %3\n}\n");
    }
    if (needs_cmpstr) {
        needs_strcmp = 1;
        insert(code, 0, strlen(code), "\ndefine i1 @cmpstr(i8* %a, i8* %b) {\nentry:\n\t%0 = call i1 @strcmp(i8* %a, i8* %b)\n\t%1 = icmp eq i1 %0, 0\n\tret i1 %1\n}\n");
    }
    if (needs_div_int) {
        insert(code, 0, strlen(code), "define double @div_int(i32 %a, i32 %b) {\nentry:\n\t%0 = sitofp i32 %a to double\n\t%1 = sitofp i32 %b to double\n\t%2 = fdiv double %0, %1\n\tret double %2\n}\n");
    }
    char* module_id = malloc(400);
    memset(module_id, 0, 400);
    snprintf(module_id, 400, "; ModuleID = '%s'\nsource_filename = \"%s\"\n", file_name, file_name);
    insert(code, 0, strlen(code), module_id);
    free(module_id);
    strcat(code, "\tret i32 0\n}\n");
    if (needs_strlen) {
        strcat(code, "declare i32 @strlen(i8*)\n");
    }
    if (needs_malloc) {
        strcat(code, "declare i8* @malloc(i32)\n");
    }
    if (needs_strcpy) {
        strcat(code, "declare i8* @strcpy(i8*, i8*)\n");
    }
    if (needs_strcpy) {
        strcat(code, "declare i8* @strcat(i8*, i8*)\n");
    }
    if (needs_scanf) {
        strcat(code, "declare i32 @scanf(i8*, ...)\n");
    }
    if (needs_printf) {
        strcat(code, "declare i32 @printf(i8*, ...)\n");
    }
    if (needs_strcmp) {
        strcat(code, "declare i1 @strcmp(i8*, i8*)\n");
    }
    heap_free_all();
    symtab_destroy();
}
