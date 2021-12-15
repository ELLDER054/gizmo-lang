#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "heap.h"
#include "codegen.h"
#include "../common/include/log.h"
#include "../front-end/ast.h"
#include "../front-end/symbols.h"
#include "../streambuf.h"

#define MAX_BUF_LEN 2048

char* current_function_return_type;
int var_c = 0; // variable count
int save_var_c = 0; // temporary variable count
int str_c = 1; // string count
char* type(Node* n);
char* allocate(Stream_buf* code, char* type, char* name);
char* load(Stream_buf* code, char* type, char* name1, char* name2);
void store(Stream_buf* code, char* type, char* name1, char* name2);

void enter_function() { // save off the variable count
    printf("Entering function, var_c = %d, save_var_c = %d\n", var_c, save_var_c);
    save_var_c = var_c;
    var_c = 0;
}

void leave_function() { // restore the variable count
    printf("Leaving function, var_c = %d, save_var_c = %d\n", var_c, save_var_c);
    var_c = save_var_c;
    save_var_c = 0;
}

char* get_type_from(char* str) { // gets the type of an array
    char* type = heap_alloc(100);
    for (int i = 0; i < strlen(str) - 2; i++) {
        char c = str[i];
        strncat(type, &c, 1);
    }
    return type;
}

char* str_format(const char* fmt, ...) { // returns a formatted string
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

char* types(char* t) { // converts a gizmo type to an llvm type
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

void insert(Stream_buf* buf, int pos, int size, char* str) { // inserts str at index pos of buf
    if (pos >= buf->index) {
        printf("WARNING: not supported yet\n");
        return;
    }

    char* temp = malloc(buf->len + strlen(str));
    memset(temp, 0, buf->len + strlen(str));

    // copy pre
    memcpy(temp, buf->buf, pos);

    // insert new
    memcpy(temp+pos, str, strlen(str));

    // copy post
    memcpy(temp+pos+strlen(str), buf->buf+pos, buf->index - pos);

    // if we need to allocate more space
    if (strlen(str) > buf->len - buf->index) {
        buf->len += strlen(str);
        buf->buf = realloc(buf->buf, buf->len);
    }

    // copy temp to stream buf
    memcpy(buf->buf, temp, strlen(temp));
    buf->index += strlen(str);
    free(temp);
}

// These variables tell us whether or not we use certain llvm functions
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
int needs_memcpy = 0;

int label_c = 0; // label count

char* find_operation_llvm(char* oper, char* t) { // converts gizmo operators to llvm commands
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

int gizmo_strlen(char* str) { // gets the length of a special kind of string passed from the scanner
    int b_c = 0;
    int pos = 0;
    int str_len = strlen(str);
    for (pos = 0; pos < str_len; pos++) {
        char c = str[pos];
        if (c == '\\') {
            b_c++;
        }
    }
    return pos - (2 * b_c);
}

char* generate_expression_llvm(Node* n, char* expr_type, Stream_buf* c);

char* generate_operation_llvm(Node* n, char* expr_type, Stream_buf* c) { // generates llvm code for an operation
    Operator_node* oper_n = ((Operator_node*) n);
    char* l = generate_expression_llvm(oper_n->left, expr_type, c);
    char* r = generate_expression_llvm(oper_n->right, expr_type, c);
    char* op_name = str_format("%%%d", var_c++);
    char* oper_llvm = find_operation_llvm(oper_n->oper, types(type(oper_n->left)));
    Stream_buf_append_str(c, str_format("\t%s = %s %s %s, ", op_name, oper_llvm, types(type(oper_n->left)), l));
    if (oper_llvm[strlen(oper_llvm) - 1] == '(') {
        Stream_buf_append_str(c, types(type((oper_n->left))));
        Stream_buf_append_str(c, " ");
    }
    Stream_buf_append_str(c, r);
    if (oper_llvm[strlen(oper_llvm) - 1] == '(') {
        Stream_buf_append_str(c, ")");
    }
    Stream_buf_append_str(c, "\n");
    return op_name;
}

int in_possible_escapes(char c) { // tells whether c is an escape character or not
    return c == 'n' || c == 'r' || c == 't' || c == 'b' || c == '\\';
}

char* generate_expression_llvm(Node* n, char* expr_type, Stream_buf* c) { // generates llvm code for an expression
    log_trace("type: %d\n", n->n_type);
    if (n->n_type == INTEGER_NODE) {
        return str_format("%d", ((Integer_node*) n)->value);
    } if (n->n_type == ID_NODE) {
        if (((Identifier_node*) n)->type[strlen(((Identifier_node*) n)->type) - 1] != ']') {
            char* id = str_format("%%%d", var_c++);
            load(c, types(((Identifier_node*) n)->type), id, str_format("%%%s", ((Identifier_node*) n)->codegen_name));
            return id;
        } else {
            return str_format("%%%s", ((Identifier_node*) n)->codegen_name);
        }
    } else if (n->n_type == CHAR_NODE) {
        return str_format("%d", (int)(((Char_node*) n)->value));
    } else if (n->n_type == STRING_NODE) {
        insert(c, 0, c->len, str_format("@.str.%d = constant [%d x i8] c\"%s\"\n", str_c, gizmo_strlen(((String_node*) n)->value), ((String_node*) n)->value));
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
            char* arg = generate_expression_llvm(((Func_call_node*) n)->args[i], type(((Func_call_node*) n)->args[i]), c);
            char* extra_name = str_format("%%%d", var_c++);
            allocate(c, types(type(((Func_call_node*) n)->args[i])), extra_name);
            store(c, types(type(((Func_call_node*) n)->args[i])), arg, extra_name);
            strcat(arg_code, types(type(((Func_call_node*) n)->args[i])));
            strcat(arg_code, "* ");
            strcat(arg_code, extra_name);
            if (i + 1 < ((Func_call_node*) n)->args_len) {
                strcat(arg_code, ", ");
            }
        }
        char* func_call_name = str_format("%%%d", var_c++);
        snprintf(call, 1024, "\t%s = call %s @%s(%s)\n", func_call_name, strcmp(types(((Func_call_node*) n)->type), "%.arr") == 0 ? "%.arr*" : types(((Func_call_node*) n)->type), ((Func_call_node*) n)->name, arg_code);
        Stream_buf_append_str(c, call);
        free(arg_code);
        free(call);
        return func_call_name;
    } else if (n->n_type == LEN_NODE) {
        Func_call_node* len = (Func_call_node*) n;
        char* len_value = generate_expression_llvm(len->args[0], type(len->args[0]), c);
        if (type(len->args[0])[strlen(type(len->args[0])) - 1] == ']') {
            char* extra_name = str_format("%%%d", var_c++);
            char* len_name = str_format("%%%d", var_c++);
            printf("%d: var_c is %d\n", __LINE__, var_c);
            Stream_buf_append_str(c, str_format("\t%s = getelementptr inbounds %%.arr, %%.arr* %s, i32 0, i32 0\n", extra_name, len_value));
            load(c, str_format("i32"), len_name, extra_name);
            return len_name;
        } else if (strcmp(type(len->args[0]), "string") == 0) {
            char* len_name = str_format("%%%d", var_c++);
            needs_strlen = 1;
            Stream_buf_append_str(c, str_format("\t%s = call i32 @strlen(i8* %s)\n", len_name, len_value));
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
        allocate(c, str_format("[1024 x i8]"), func_call_name);
        Stream_buf_append_str(c, str_format("\t%s = getelementptr inbounds [1024 x i8], [1024 x i8]* %s, i32 0, i32 0\n", temp_var, func_call_name));
        char* write_name = generate_expression_llvm(((Func_call_node*) n)->args[0], "string", c);
        printf("%d: var_c is %d\n", __LINE__, var_c);
        Stream_buf_append_str(c, str_format("\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i32 0, i32 0), i8* %s)\n", write_name));
        var_c++;
        printf("%d: var_c is %d\n", __LINE__, var_c);
        Stream_buf_append_str(c, str_format("\tcall i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i32 0, i32 0), i8* %s)\n", temp_var));
        var_c++;
        char* temp_var2 = str_format("%%%d", var_c++);
        allocate(c, str_format("i8*"), temp_var2);
        store(c, str_format("i8*"), temp_var, temp_var2);
        char* last_temp_var = str_format("%%%d", var_c++);
        load(c, str_format("i8*"), last_temp_var, temp_var2);
        return last_temp_var;
    } else if (n->n_type == WRITE_NODE) {
        return "0";
    }
    
    return generate_operation_llvm(n, expr_type, c);
}

char* allocate(Stream_buf* code, char* type, char* name) {
    if (name == NULL) {
        name = str_format("%%%d", var_c++);
    }
    Stream_buf_append_str(code, str_format("\t%s = alloca %s\n", name, type));
    return name;
}

void store(Stream_buf* code, char* type, char* name1, char* name2) {
    Stream_buf_append_str(code, str_format("\tstore %s %s, %s* %s\n", type, name1, type, name2));
}

char* load(Stream_buf* code, char* type, char* name1, char* name2) {
    if (name1 == NULL) {
        name1 = str_format("%%%d", var_c++);
    }
    Stream_buf_append_str(code, str_format("\t%s = load %s, %s* %s\n", name1, type, type, name2));
    return name1;
}

void llvm_generate_statement(Node* n, Stream_buf* code) { // generates llvm code for a statement
        printf("%d: var_c is %d\n", __LINE__, var_c);
        if (n->n_type == VAR_DECLARATION_NODE) {
            Var_declaration_node* v = (Var_declaration_node*) n;
            char* var_name = generate_expression_llvm(v->value, v->type, code);
            allocate(code, types(v->type), str_format("%%%s", v->codegen_name));
            store(code, types(v->type), var_name, str_format("%%%s", v->codegen_name));
        } else if (n->n_type == VAR_ASSIGN_NODE) {
            Var_assignment_node* v = (Var_assignment_node*) n;
            char* var_name = generate_expression_llvm(v->value, type(v->value), code);
            store(code, types(type(v->value)), var_name, str_format("%%%s", v->codegen_name));
        } else if (n->n_type == SKIP_NODE) {
            Stream_buf_append_str(code, "\t");
            Stream_buf_append_str(code, ((Skip_node*) n)->code);
            var_c++;
        } else if (n->n_type == WHILE_NODE) {
            While_loop_node* wh = (While_loop_node*) n;
            char* while_name = generate_expression_llvm(wh->condition, type(wh->condition), code);
            Stream_buf_append_str(code, str_format("\tbr i1 %s, label %%%s, label %%%s\n%s:\n", while_name, wh->begin_cgid, wh->end_cgid, wh->begin_cgid));
            llvm_generate_statement(wh->body, code);
            char* while_name2 = generate_expression_llvm(wh->condition, type(wh->condition), code);
            Stream_buf_append_str(code, str_format("\tbr i1 %s, label %%%s, label %%%s\n%s:\n", while_name2, wh->begin_cgid, wh->end_cgid, wh->end_cgid));
        } else if (n->n_type == IF_NODE) {
            If_node* i = (If_node*) n;
            char* if_name = generate_expression_llvm(i->condition, type(i->condition), code);
            Stream_buf_append_str(code, str_format("\tbr i1 %s, label %%%s, label %%%s\n%s:\n", if_name, i->else_body == NULL? i->begin_cgid : i->else_cgid, i->end_cgid, i->begin_cgid));
            llvm_generate_statement(i->body, code);
            Stream_buf_append_str(code, str_format("\tbr label %%%s", i->end_cgid));
            if (i->else_body != NULL) {
                Stream_buf_append_str(code, str_format("\n%s:\n", i->else_cgid));
                llvm_generate_statement(i->else_body, code);
                Stream_buf_append_str(code, str_format("\tbr label %%%s\n", i->end_cgid));
            }
            Stream_buf_append_str(code, str_format("\n%s:\n", i->end_cgid));
        } else if (n->n_type == WRITE_NODE) {
            Func_call_node* func = (Func_call_node*) n;
            needs_printf = 1;
            char* write_arg_name = generate_expression_llvm(func->args[0], type(func->args[0]), code);
            if (strcmp(type(func->args[0]), "int") == 0) {
                needs_num_const = 1;
                printf("%d: var_c is %d\n", __LINE__, var_c);
                Stream_buf_append_str(code, str_format("\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.num, i32 0, i32 0), i32 %s)", write_arg_name));
            } else if (strcmp(type(func->args[0]), "string") == 0) {
                needs_str_const = 1;
                printf("%d: var_c is %d\n", __LINE__, var_c);
                Stream_buf_append_str(code, str_format("\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i32 0, i32 0), i8* %s)", write_arg_name));
            } else if (strcmp(type(func->args[0]), "bool") == 0) {
                needs_true_const = 1;
                needs_false_const = 1;
                char* t = str_format("true%d", label_c++);
                char* f = str_format("false%d", label_c++);
                char* end = str_format("end%d", label_c++);
                var_c++;
                Stream_buf_append_str(code, "\tbr i1 ");
                Stream_buf_append_str(code, write_arg_name);
                Stream_buf_append_str(code, ", label %");
                Stream_buf_append_str(code, t);
                Stream_buf_append_str(code, ", label %");
                Stream_buf_append_str(code, f);
                Stream_buf_append_str(code, "\n");
                Stream_buf_append_str(code, t);
                printf("%d: var_c is %d\n", __LINE__, var_c);
                Stream_buf_append_str(code, ":\n\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.true, i32 0, i32 0))\n\tbr label %");
                Stream_buf_append_str(code, end);
                Stream_buf_append_str(code, "\n");
                Stream_buf_append_str(code, f);
                printf("%d: var_c is %d\n", __LINE__, var_c);
                Stream_buf_append_str(code, ":\n\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @.false, i32 0, i32 0))\n\tbr label %");
                Stream_buf_append_str(code, end);
                Stream_buf_append_str(code, "\n");
                Stream_buf_append_str(code, end);
                Stream_buf_append_str(code, ":");
            } else if (strcmp(type(func->args[0]), "char") == 0) {
                needs_chr_const = 1;
                printf("%d: var_c is %d\n", __LINE__, var_c);
                Stream_buf_append_str(code, str_format("\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.chr, i32 0, i32 0), i8 %s)", write_arg_name));
            } else if (strcmp(type(func->args[0]), "real") == 0) {
                needs_real_const = 1;
                printf("%d: var_c is %d\n", __LINE__, var_c);
                Stream_buf_append_str(code, str_format("\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.real, i32 0, i32 0), double %s)", write_arg_name));
            }
            Stream_buf_append_str(code, "\n");
            var_c++;
        } else if (n->n_type == READ_NODE) {
            Stream_buf_append_str(code, "\n");
        } else if (n->n_type == BLOCK_NODE) {
            for (int i = 0; i < ((Block_node*) n)->ssize; i++) {
                llvm_generate_statement(((Block_node*) n)->statements[i], code);
            }
        } else if (n->n_type == FUNC_CALL_NODE) {
            char* arg_code = malloc(100);
            memset(arg_code, 0, 100);
            for (int i = 0; i < ((Func_call_node*) n)->args_len; i++) {
                char* arg = generate_expression_llvm(((Func_call_node*) n)->args[i], type(((Func_call_node*) n)->args[i]), code);
                char* extra_name = str_format("%%%d", var_c++);
                allocate(code, types(type(((Func_call_node*) n)->args[i])), extra_name);
                store(code, types(type(((Func_call_node*) n)->args[i])), arg, extra_name);
                strcat(arg_code, types(type(((Func_call_node*) n)->args[i])));
                strcat(arg_code, "* ");
                strcat(arg_code, extra_name);
                if (i + 1 < ((Func_call_node*) n)->args_len) {
                    strcat(arg_code, ", ");
                }
            }
            Stream_buf_append_str(code, str_format("\tcall %s @%s(%s)", types(symtab_find_global(((Func_call_node*) n)->name, "func")->type), ((Func_call_node*) n)->name, arg_code));
            free(arg_code);
            var_c++;
        } else if (n->n_type == FUNC_DECL_NODE) {
            Stream_buf* mini_code = new_Stream_buf(NULL, 5000);
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
            Stream_buf_append_str(mini_code, str_format("define %s @%s(%s) {\nentry:\n", strcmp(types(((Func_decl_node*) n)->type), "%.arr") == 0 ? "%.arr*" : types(((Func_decl_node*) n)->type), ((Func_decl_node*) n)->name, arg_code));
            strcpy(current_function_return_type, ((Func_decl_node*) n)->type);
            enter_function();
            llvm_generate_statement(((Func_decl_node*) n)->body, mini_code);
            leave_function();
            strcpy(current_function_return_type, "");
            Stream_buf_append_str(mini_code, "}\n");
            insert(code, 0, code->len, ((char*) (mini_code->buf)));
            free_Stream_buf(mini_code);
            free(arg_code);
        } else if (n->n_type == RET_NODE) {
            char* ret_name = generate_expression_llvm(((Return_node*) n)->expr, current_function_return_type, code);
            var_c++;
            Stream_buf_append_str(code, str_format("\tret %s %s\n", strcmp(types(current_function_return_type), "%.arr") == 0 ? "%.arr*" : types(current_function_return_type), ret_name));
        } else {
            fprintf(stderr, "gizmo: This feature (%d) is either not yet implemented in the back-end or there is an internal compiler error\nPlease report this error, along with the number in the parenthesis, to the developers at gizmo@gizmolang.org\n", n->n_type);
            exit(-1);
        }
}

void generate(Node** ast, int size, Stream_buf* code, char* file_name) { // creates the full llvm module
    current_function_return_type = malloc(100);
    memset(current_function_return_type, 0, 100);
    Stream_buf_append_str(code, "\ndefine i32 @main() {\nentry:\n");
    heap_init();
    for (int i = 0; i < size; i++) {
        Node* n = ast[i];
        if (n == NULL) {
            break;
        }
        llvm_generate_statement(n, code);
    }
    if (needs_chr_const) {
        insert(code, 0, code->len, "@.chr = constant [3 x i8] c\"%c\\00\"\n");
    }
    if (needs_str_const) {
        insert(code, 0, code->len, "@.str = constant [3 x i8] c\"%s\\00\"\n");
    }
    if (needs_real_const) {
        insert(code, 0, code->len, "@.real = constant [3 x i8] c\"%f\\00\"\n");
    }
    if (needs_num_const) {
        insert(code, 0, code->len, "@.num = constant [3 x i8] c\"%d\\00\"\n");
    }
    if (needs_true_const) {
        insert(code, 0, code->len, "@.true = constant [6 x i8] c\"true\\0A\\00\"");
    }
    if (needs_false_const) {
        insert(code, 0, code->len, "@.false = constant [7 x i8] c\"false\\0A\\00\"");
    }
    if (needs_constr) {
        needs_malloc = 1;
        needs_strlen = 1;
        needs_strcpy = 1;
        needs_strcat = 1;
        insert(code, 0, code->len, "\ndefine i8* @constr(i8* %a, i8* %b) {\nentry:\n\t%0 = call i32 @strlen(i8* %a)\n\t%1 = call i32 @strlen(i8* %b)\n\t%2 = add i32 %0, %1\n\t%3 = call i8* @malloc(i32 %2)\n\tcall i8* @strcpy(i8* %3, i8* %a)\n\tcall i8* @strcat(i8* %3, i8* %b)\n\tret i8* %3\n}\n");
    }
    if (needs_cmpstr) {
        needs_strcmp = 1;
        insert(code, 0, code->len, "\ndefine i1 @cmpstr(i8* %a, i8* %b) {\nentry:\n\t%0 = call i32 @strcmp(i8* %a, i8* %b)\n\t%1 = icmp eq i32 %0, 0\n\tret i1 %1\n}\n");
    }
    if (needs_cmpnstr) {
        needs_strcmp = 1;
        insert(code, 0, code->len, "\n i1 @cmpnstr(i8* %a, i8* %b) {\nentry:\n\t%0 = call i32 @strcmp(i8* %a, i8* %b)\n\t%1 = icmp ne i32 %0, 0\n\tret i1 %1\n}\n");
    }
    if (needs_div_int) {
        insert(code, 0, code->len, "define double @div_int(i32 %a, i32 %b) {\nentry:\n\t%0 = sitofp i32 %a to double\n\t%1 = sitofp i32 %b to double\n\t%2 = fdiv double %0, %1\n\tret double %2\n}\n");
    }
    char* module_id = str_format("; ModuleID = '%s'\nsource_file_name = \"%s\"\n\n", file_name, file_name);
    Stream_buf_append_str(code, "\tret i32 0\n}\n");
    if (needs_strlen) {
        Stream_buf_append_str(code, "declare i32 @strlen(i8*)\n");
    }
    if (needs_malloc) {
        Stream_buf_append_str(code, "declare i8* @malloc(i32)\n");
    }
    if (needs_strcpy) {
        Stream_buf_append_str(code, "declare i8* @strcpy(i8*, i8*)\n");
    }
    if (needs_strcpy) {
        Stream_buf_append_str(code, "declare i8* @strcat(i8*, i8*)\n");
    }
    if (needs_scanf) {
        Stream_buf_append_str(code, "declare i32 @scanf(i8*, ...)\n");
    }
    if (needs_printf) {
        Stream_buf_append_str(code, "declare i32 @printf(i8*, ...)\n");
    }
    if (needs_strcmp) {
        Stream_buf_append_str(code, "declare i32 @strcmp(i8*, i8*)\n");
    }
    if (needs_memcpy) {
        Stream_buf_append_str(code, "declare i32 @memcpy(i8*, i8*, i32)\n");
    }
    heap_free_all();
    symtab_destroy();
}
