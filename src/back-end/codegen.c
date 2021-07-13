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

char* get_type_from(char* str) {
    char* type = heap_alloc(100);
    for (int i = 0; i < strlen(str) - 2; i++) {
        char c = str[i];
        strncat(type, &c, 1);
    }
    return type;
}

char* str_format(const char* fmt, ...) {
	int size = 0;
    char *p = NULL;
    va_list ap;

    va_start(ap, fmt);
    log_trace("make formatted with fmt: %s, size: %d\n", fmt, size);
    size = vsnprintf(p, size, fmt, ap);
    va_end(ap);

    if (size < 0)
  	    return NULL;

    size++;             /* For '\0' */
    p = heap_alloc(size);
    if (p == NULL)
    	return NULL;

    va_start(ap, fmt);
    size = vsnprintf(p, size, fmt, ap);
    va_end(ap);

    if (size < 0) {
	    return NULL;
    }

    return p;
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
    } else if (t[strlen(t) - 1] == ']') {
        return "%.arr";
    }
    return "";
}

void insert(char* buf, int pos, int size, char* str) {
    char* temp = malloc(MAX_BUF_LEN * size);
    memset(temp, 0, MAX_BUF_LEN * size);
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
int needs_cmpnstr = 0;
int needs_strcmp = 0;
int needs_div_int = 0;
int needs_str_const = 0;
int needs_true_const = 0;
int needs_false_const = 0;
int needs_num_const = 0;
int needs_real_const = 0;
int needs_chr_const = 0;
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
            return "mul nsw";
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
            needs_cmpnstr = 1;
            return "call i1 @cmpnstr(";
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

char* generate_expression_asm(Node* n, char* expr_type, char* c);

char* generate_operation_asm(Node* n, char* expr_type, char* c) {
    char* l = generate_expression_asm(((Operator_node*) n)->left, expr_type, c);
    char* r = generate_expression_asm(((Operator_node*) n)->right, expr_type, c);
    char* op_name = str_format("%%%d", var_c++);
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
    if (strcmp(oper_asm, "call i8* @constr(") == 0 || strcmp(oper_asm, "call i1 @cmpstr(") == 0 || strcmp(oper_asm, "call i1 @cmpnstr(") == 0) {
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

int in_possible_escapes(char c) {
    return c == 'n' || c == 'r' || c == 't' || c == 'b' || c == '\\';
}

int gizmo_strlen(char* str) {
    int pos = 0;
    int len = 0;
    while (pos < strlen(str) - 3) {
        char c = str[pos];
        if (c == '\\') {
            len++;
            pos += in_possible_escapes(str[pos + 1]) ? 2 : 1;
        } else {
            len++;
            pos++;
        }
    }
    len++;
    return len;
}

char* generate_expression_asm(Node* n, char* expr_type, char* c) {
    log_trace("type: %d\n", n->n_type);
    if (n->n_type == INTEGER_NODE) {
        return str_format("%d", ((Integer_node*) n)->value);
    } else if (n->n_type == LIST_NODE) {
        char* end_list_name = str_format("%%%d", var_c++);
        char* list_name = str_format("%%%d", var_c++);
        char* allarr_name = str_format("%%%d", var_c++);
        char* len_name = str_format("%%%d", var_c++);
        List_node* list = (List_node*) n;
        char* len = str_format("%d", list->len);
        strcat(c, str_format("\t%s = alloca %%.arr\n\t%s = getelementptr inbounds %%.arr, %%.arr* %s, i32 0, i32 1\n\t%s = alloca [%s x %s]\n\t%s = getelementptr inbounds %%.arr, %%.arr* %s, i32 0, i32 0\n\tstore i32 %s, i32* %s\n", end_list_name, list_name, end_list_name, allarr_name, len, types(get_type_from(list->type)), len_name, end_list_name, len, len_name));
        for (int i = 0; i < list->len; i++) {
            char* element_name = generate_expression_asm(list->elements[i], type(list->elements[i]), c);
            char* name = str_format("%%%d", var_c++);
            char* current_number = str_format("%d", i);
            strcat(c, "\t");
            strcat(c, name);
            strcat(c, str_format(" = getelementptr inbounds [%s x %s], [%s x %s]* ", len, types(get_type_from(list->type)), len, types(get_type_from(list->type))));
            strcat(c, allarr_name);
            strcat(c, ", i32 0, i32 ");
            strcat(c, current_number);
            strcat(c, "\n\tstore ");
            strcat(c, types(get_type_from(list->type)));
            strcat(c, " ");
            strcat(c, element_name);
            strcat(c, ", ");
            strcat(c, types(get_type_from(list->type)));
            strcat(c, "* ");
            strcat(c, name);
            strcat(c, "\n");
        }
        char* end_getel_name = str_format("%%%d", var_c++);
        char* bitcast_name = str_format("%%%d", var_c++);
        strcat(c, str_format("\t%s = getelementptr inbounds [%s x %s], [%s x %s]* %s, i32 0, i32 0\n\t%s = bitcast %s* %s to i8*\n\tstore i8* %s, i8** %s\n", end_getel_name, len, types(get_type_from(list->type)), len, types(get_type_from(list->type)), allarr_name, bitcast_name, types(get_type_from(list->type)), end_getel_name, bitcast_name, list_name));
        return end_list_name;
    } else if (n->n_type == INDEX_NODE) {
        Index_node* index = (Index_node*) n;
        char* index_value_name = generate_expression_asm(index->expr, type(index->expr), c);
        char* index_id = generate_expression_asm(index->id, type(index->id), c);
        if (strcmp(type(n), "char") != 0) {
            char* arr_name = str_format("%%%d", var_c++);
            char* loadi8_name = str_format("%%%d", var_c++);
            char* bitcast_arr_name = str_format("%%%d", var_c++);
            char* extra_extra_name = str_format("%%%d", var_c++);
            char* extra_name = str_format("%%%d", var_c++);
            strcat(c, "\t");
            strcat(c, arr_name);
            strcat(c, str_format(" = getelementptr inbounds %%.arr, %%.arr* %s", index_id));
            strcat(c, ", i32 0, i32 1\n\t");
            strcat(c, str_format("%s = load i8*, i8** %s\n\t", loadi8_name, arr_name));
            strcat(c, bitcast_arr_name);
            strcat(c, str_format(" = bitcast i8* %s to %s*\n\t", loadi8_name, types(index->type)));
            strcat(c, extra_extra_name);
            strcat(c, str_format(" = getelementptr inbounds %s, %s* ", types(index->type), types(index->type)));
            strcat(c, bitcast_arr_name);
            strcat(c, ", i32 ");
            strcat(c, index_value_name);
            strcat(c, "\n\t");
            strcat(c, extra_name);
            strcat(c, " = load ");
            strcat(c, types(index->type));
            strcat(c, ", ");
            strcat(c, types(index->type));
            strcat(c, "* ");
            strcat(c, extra_extra_name);
            strcat(c, "\n");
            return extra_name;
        } else {
            char* arr_name = str_format("%%%d", var_c++);
            char* loadi8_name = str_format("%%%d", var_c++);
            strcat(c, arr_name);
            strcat(c, str_format(" = getelementptr inbounds %s, %s* %s", types(index->type), types(index->type), index_id));
            strcat(c, str_format(", i32 %s\n\t%s = load i8, i8* %s\n", index_value_name, loadi8_name, arr_name));
            return loadi8_name;
        }
    } if (n->n_type == ID_NODE) {
        char* id = str_format("%%%d", var_c);
        if (((Identifier_node*) n)->type[strlen(((Identifier_node*) n)->type) - 1] != ']') {
            var_c++;
            strcat(c, str_format("\t%s = load %s, %s* %%%s\n", id, types(((Identifier_node*) n)->type), types(((Identifier_node*) n)->type), ((Identifier_node*) n)->codegen_name));
        } else {
            return str_format("%%%s", ((Identifier_node*) n)->codegen_name);
        }
        return id;
    } else if (n->n_type == CHAR_NODE) {
        return str_format("%d", (int)(((Char_node*) n)->value));
    } else if (n->n_type == STRING_NODE) {
        insert(c, 0, strlen(c), str_format("@.str.%d = constant [%d x i8] c\"%s\"\n", str_c, gizmo_strlen(((String_node*) n)->value), ((String_node*) n)->value));
        return str_format("bitcast ([%d x i8]* @.str.%d to i8*)", gizmo_strlen(((String_node*) n)->value), str_c++);
    } else if (n->n_type == REAL_NODE) {
        return str_format("%f", ((Real_node*) n)->value);
    } else if (n->n_type == BOOL_NODE) {
        return str_format("%d", ((Boolean_node*) n)->value);
    } else if (n->n_type == FUNC_CALL_NODE) {
        char* call = malloc(1024);
        memset(call, 0, 1024);
        char* arg_code = malloc(100);
        memset(arg_code, 0, 100);
        for (int i = 0; i < ((Func_call_node*) n)->args_len; i++) {
            char* arg = generate_expression_asm(((Func_call_node*) n)->args[i], type(((Func_call_node*) n)->args[i]), c);
            strcat(c, "\t");
            char* extra_name = str_format("%%%d", var_c++);
            strcat(c, extra_name);
            strcat(c, " = alloca ");
            strcat(c, types(type(((Func_call_node*) n)->args[i])));
            strcat(c, "\n\tstore ");
            strcat(c, types(type(((Func_call_node*) n)->args[i])));
            strcat(c, " ");
            strcat(c, arg);
            strcat(c, ", ");
            strcat(c, types(type(((Func_call_node*) n)->args[i])));
            strcat(c, "* ");
            strcat(c, extra_name);
            strcat(arg_code, types(type(((Func_call_node*) n)->args[i])));
            strcat(arg_code, "* ");
            strcat(arg_code, extra_name);
            if (i + 1 < ((Func_call_node*) n)->args_len) {
                strcat(arg_code, ", ");
            }
        }
        char* func_call_name = str_format("%%%d", var_c++);
        snprintf(call, 1024, "\t%s = call %s @%s(%s)\n", func_call_name, types(symtab_find_global(((Func_call_node*) n)->name, "func")->type), ((Func_call_node*) n)->name, arg_code);
        strcat(c, call);
        free(arg_code);
        return func_call_name;
    } else if (n->n_type == LEN_NODE) {
        Func_call_node* len = (Func_call_node*) n;
        char* len_value = generate_expression_asm(len->args[0], type(len->args[0]), c);
        if (type(len->args[0])[strlen(type(len->args[0])) - 1] == ']') {
            char* extra_name = str_format("%%%d", var_c++);
            char* len_name = str_format("%%%d", var_c++);
            strcat(c, str_format("\t%s = getelementptr inbounds %%.arr, %%.arr* %s, i32 0, i32 0\n\t%s = load i32, i32* %s\n", extra_name, len_value, len_name, extra_name));
            return len_name;
        } else if (strcmp(type(len->args[0]), "string") == 0) {
            char* len_name = str_format("%%%d", var_c++);
            needs_strlen = 1;
            strcat(c, str_format("\t%s = call i32 @strlen(i8* %s)\n", len_name, len_value));
            return len_name;
        }
    } else if (n->n_type == READ_NODE) {
        needs_scanf = 1;
        if (strcmp(type(((Func_call_node*) n)->args[0]), "string")) {
            fprintf(stderr, "error: Argument 1 of the 'read' function must be a string\n");
        }
        needs_str_const = 1;
        char* func_call_name = str_format("%%%d", var_c++);
        char* temp_var = str_format("%%%d", var_c++);
        strcat(c, "\t");
        strcat(c, func_call_name);
        strcat(c, " = alloca [1024 x i8], align 8\n\t"); // LIMIT
        strcat(c, temp_var);
        strcat(c, " = getelementptr inbounds [1024 x i8], [1024 x i8]* "); // LIMIT
        strcat(c, func_call_name);
        strcat(c, ", i32 0, i32 0\n\t");
        char* write_name = generate_expression_asm(((Func_call_node*) n)->args[0], "string", c);
        strcat(c, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i32 0, i32 0), i8* ");
        strcat(c, write_name);
        var_c++;
        strcat(c, ")\n\t");
        strcat(c, "call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i32 0, i32 0), i8* ");
        strcat(c, temp_var);
        var_c++;
        char* temp_var2 = str_format("%%%d", var_c++);
        strcat(c, ")\n\t");
        strcat(c, temp_var2);
        strcat(c, " = alloca i8*\n\tstore i8*");
        strcat(c, temp_var);
        strcat(c, ", i8** ");
        strcat(c, temp_var2);
        strcat(c, "\n");
        char* last_temp_var = str_format("%%%d", var_c++);
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
            char* var_name = generate_expression_asm(v->value, v->type, code);
            if (v->type[strlen(v->type) - 1] == ']') {
                char* extra_name = str_format("%%%d", var_c++);
                strcat(code, str_format("\t%%%s = alloca %s\n\t%s = load %%.arr, %%.arr* %s\n\tstore %s %s, %s* %%%s\n", v->codegen_name, types(v->type), extra_name, var_name, types(v->type), extra_name, types(v->type), v->codegen_name));
            } else {
                strcat(code, str_format("\t%%%s = alloca %s\n\tstore %s %s, %s* %%%s\n", v->codegen_name, types(v->type), types(v->type), var_name, types(v->type), v->codegen_name));
            }
        } else if (n->n_type == VAR_ASSIGN_NODE) {
            Var_assignment_node* v = (Var_assignment_node*) n;
            char* var_name = generate_expression_asm(v->value, type(v->value), code);
            strcat(code, str_format("\tstore %s %s, %s* %%%s\n", types(type(v->value)), var_name, types(type(v->value)), v->codegen_name));
        } else if (n->n_type == SKIP_NODE) {
            strcat(code, "\t");
            strcat(code, ((Skip_node*) n)->code);
            var_c++;
        } else if (n->n_type == WHILE_NODE) {
            While_loop_node* wh = (While_loop_node*) n;
            char* while_name = generate_expression_asm(wh->condition, type(wh->condition), code);
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
            char* while_name2 = generate_expression_asm(wh->condition, type(wh->condition), code);
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
            If_node* i = (If_node*) n;
            char* if_name = generate_expression_asm(i->condition, type(i->condition), code);
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
            needs_printf = 1;
            char* write_arg_name = generate_expression_asm(func->args[0], type(func->args[0]), code);
            if (strcmp(type(func->args[0]), "int") == 0) {
                needs_num_const = 1;
                strcat(code, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.num, i32 0, i32 0), i32 ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            } else if (strcmp(type(func->args[0]), "string") == 0) {
                needs_str_const = 1;
                strcat(code, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i32 0, i32 0), i8* ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            } else if (strcmp(type(func->args[0]), "bool") == 0) {
                needs_true_const = 1;
                needs_false_const = 1;
                char* t = str_format("true%d", bool_c);
                char* f = str_format("false%d", bool_c++);
                char* end = str_format("end%d", end_c++);
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
            } else if (strcmp(type(func->args[0]), "char") == 0) {
                needs_chr_const = 1;
                strcat(code, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.chr, i32 0, i32 0), i8 ");
                strcat(code, write_arg_name);
                strcat(code, ")");
            } else if (strcmp(type(func->args[0]), "real") == 0) {
                needs_real_const = 1;
                strcat(code, "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.real, i32 0, i32 0), double ");
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
            char* arg_code = malloc(100);
            memset(arg_code, 0, 100);
            for (int i = 0; i < ((Func_call_node*) n)->args_len; i++) {
                char* arg = generate_expression_asm(((Func_call_node*) n)->args[i], type(((Func_call_node*) n)->args[i]), code);
                strcat(code, "\t");
                if (type(((Func_call_node*) n)->args[i])[strlen(type(((Func_call_node*) n)->args[i])) - 1] != ']') {
                    char* extra_name = str_format("%%%d", var_c++);
                    strcat(code, extra_name);
                    strcat(code, " = alloca ");
                    strcat(code, types(type(((Func_call_node*) n)->args[i])));
                    strcat(code, str_format("\n\tstore %s %s, %s* %s\n", types(type(((Func_call_node*) n)->args[i])), arg, types(type(((Func_call_node*) n)->args[i])), extra_name));
                    strcat(arg_code, types(type(((Func_call_node*) n)->args[i])));
                    strcat(arg_code, "* ");
                    strcat(arg_code, extra_name);
                } else {
                    strcat(arg_code, str_format("%%.arr* %s", arg));
                }
                if (i + 1 < ((Func_call_node*) n)->args_len) {
                    strcat(arg_code, ", ");
                }
            }
            strcat(code, "\tcall ");
            strcat(code, types(symtab_find_global(((Func_call_node*) n)->name, "func")->type));
            strcat(code, " @");
            strcat(code, ((Func_call_node*) n)->name);
            strcat(code, "(");
            strcat(code, arg_code);
            strcat(code, ")");
            free(arg_code);
            var_c++;
        } else if (n->n_type == FUNC_DECL_NODE) {
            char* mini_code = malloc(5000);
            memset(mini_code, 0, 5000);
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
            char* ret_name = generate_expression_asm(((Return_node*) n)->expr, current_function_return_type, code);
            var_c++;
            strcat(code, str_format("\tret %s %s\n", types(current_function_return_type), ret_name));
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
    if (needs_chr_const) {
        insert(code, 0, strlen(code), "@.chr = constant [3 x i8] c\"%c\\00\"\n");
    }
    if (needs_str_const) {
        insert(code, 0, strlen(code), "@.str = constant [3 x i8] c\"%s\\00\"\n");
    }
    if (needs_real_const) {
        insert(code, 0, strlen(code), "@.real = constant [3 x i8] c\"%f\\00\"\n");
    }
    if (needs_num_const) {
        insert(code, 0, strlen(code), "@.num = constant [3 x i8] c\"%d\\00\"\n");
    }
    if (needs_true_const) {
        insert(code, 0, strlen(code), "@.true = constant [6 x i8] c\"true\\0A\\00\"");
    }
    if (needs_false_const) {
        insert(code, 0, strlen(code), "@.false = constant [7 x i8] c\"false\\0A\\00\"");
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
        insert(code, 0, strlen(code), "\ndefine i1 @cmpstr(i8* %a, i8* %b) {\nentry:\n\t%0 = call i32 @strcmp(i8* %a, i8* %b)\n\t%1 = icmp eq i32 %0, 0\n\tret i1 %1\n}\n");
    }
    if (needs_cmpnstr) {
        needs_strcmp = 1;
        insert(code, 0, strlen(code), "\n i1 @cmpnstr(i8* %a, i8* %b) {\nentry:\n\t%0 = call i32 @strcmp(i8* %a, i8* %b)\n\t%1 = icmp ne i32 %0, 0\n\tret i1 %1\n}\n");
    }
    if (needs_div_int) {
        insert(code, 0, strlen(code), "define double @div_int(i32 %a, i32 %b) {\nentry:\n\t%0 = sitofp i32 %a to double\n\t%1 = sitofp i32 %b to double\n\t%2 = fdiv double %0, %1\n\tret double %2\n}\n");
    }
    char* module_id = malloc(400);
    memset(module_id, 0, 400);
    snprintf(module_id, 400, "; ModuleID = '%s'\nsource_filename = \"%s\"\n\n%%.arr = type {i32, i8*}\n", file_name, file_name);
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
        strcat(code, "declare i32 @strcmp(i8*, i8*)\n");
    }
    heap_free_all();
    symtab_destroy();
}
