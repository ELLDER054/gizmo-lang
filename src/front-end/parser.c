#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#include "log.h"
#include "scanner.h"
#include "symbols.h"

int id_c = 0;

int ind = 0;
Token tokens[1024];
void print_node(FILE* f, Node* n);
void compile(char* code, char* output, char* in, char* out, char* file_name);

// begin helper functions

void repeat_char(char c, int n, char* string) {
    for (int i = 0; i < n; i++) {
        strncat(string, &c, 1);
    }
}

int tokslen(Token* tokens) {
    int len = 0;
    for (int i = 0; i < 1024; i++) {
        if (tokens[i].type < 200 || tokens[i].type > 236) {
            break;
        }
        len++;
    }
    return len;
}

void consume(TokenType type, char* err, char* buffer) {
    if (ind >= tokslen(tokens)) {
        char specifier[MAX_LINE_LEN] = "";
        repeat_char(' ', tokens[ind - 1].col + strlen(tokens[ind - 1].value), specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\n%s%s\n%s\n", tokens[ind - 1].lineno, err, tokens[ind - 1].line, specifier);
        exit(0);
    }
    if (tokens[ind].type == type) {
        strncpy(buffer, tokens[ind++].value, MAX_NAME_LEN);
        return;
    }
    char specifier[100] = "";
    repeat_char(' ', tokens[ind - 1].col + strlen(tokens[ind - 1].value), specifier);
    strncat(specifier, "^", 2);
    printf("On line %d:\n%s%s\n%s\n", tokens[ind - 1].lineno, err, tokens[ind - 1].line, specifier);
    exit(0);
}

char* expect_type(TokenType type) {
    if (ind >= tokslen(tokens)) {
        return NULL;
    }
    if (tokens[ind].type == type) {
        return tokens[ind++].value;
    }
    return NULL;
}

TokenType expect_value(char* val) {
    if (ind >= tokslen(tokens)) {
        return 0;
    }
    if (!strcmp(tokens[ind].value, val)) {
        return tokens[ind++].type;
    }
    return 0;
}

// end helper functions

Node* expression(int start);
Node* term(int start);
Node* term2(int start);
Node* factor(int start);

// begin expressions parsing

Node* incomplete_function_call(int start);

char* type(Node* n) {
    if (n == NULL) {
        return NULL;
    }
    switch (n->n_type) {
        case OPERATOR_NODE:
            if (((Operator_node*) n)->left->n_type == INTEGER_NODE && ((Operator_node*) n)->right->n_type == INTEGER_NODE && *((Operator_node*) n)->oper == '/') {
                if (((Integer_node*)((Operator_node*) n)->right)->value == 0) {
                    printf("can't divide by zero!\n");
                    exit(1);
                }
                if (((Integer_node*) ((Operator_node*) n)->left)->value % ((Integer_node*) ((Operator_node*) n)->right)->value == 0) {
                    return "int";
                }
                return "real";
            }
            return type(((Operator_node*) n)->left);
        case INTEGER_NODE:
            return "int";
        case BLOCK_NODE:
            break;
        case CHAR_NODE:
            return "char";
        case STRING_NODE:
            return "string";
        case REAL_NODE:
            return "real";
        case VAR_DECLARATION_NODE:
            break;
        case ID_NODE:
            return ((Identifier_node*) n)->type;
        case FUNC_CALL_NODE:
        case READ_NODE:
        case WRITE_NODE:
           return symtab_find_global(((Func_call_node*) n)->name, "func")->type; 
        case NODE_NODE:
            break;
    }
    return NULL;
}

void check_type(int start, Node* left, Node* right, char* oper) {
    if (*oper == '+') {
        if (!strcmp(type(left), "int")) {
            if (strcmp(type(right), "int")) {
                char specifier[MAX_LINE_LEN] = "";
                repeat_char(' ', tokens[ind - 1].col + strlen(tokens[ind - 1].value), specifier);
                strncat(specifier, "^", 2);
                printf("On line %d:\nExpected Integer on right side of expression\n%s\n%s\n", tokens[ind - 1].lineno, tokens[ind - 1].line, specifier);
                exit(0);
            }
        }
        else if (!strcmp(type(left), "string")) {
            if (strcmp(type(right), "string")) {
                char specifier[MAX_LINE_LEN] = "";
                repeat_char(' ', tokens[ind - 1].col + strlen(tokens[ind - 1].value), specifier);
                strncat(specifier, "^", 2);
                printf("On line %d:\nExpected string on right side of expression\n%s\n%s\n", tokens[ind - 1].lineno, tokens[ind - 1].line, specifier);
                exit(0);
            }
        }
        else if (!strcmp(type(left), "real")) {
            if (strcmp(type(right), "real")) {
                char specifier[MAX_LINE_LEN] = "";
                repeat_char(' ', tokens[ind - 1].col + strlen(tokens[ind - 1].value), specifier);
                strncat(specifier, "^", 2);
                printf("On line %d:\nExpected real on right side of expression\n%s\n%s\n", tokens[ind - 1].lineno, tokens[ind - 1].line, specifier);
                exit(0);
            }
        } else {
            char specifier[MAX_LINE_LEN] = "";
            repeat_char(' ', tokens[start].col, specifier);
            strncat(specifier, "^", 2);
            printf("On line %d:\nInvalid type `%s` on left side of expression\n%s\n%s\n", tokens[start].lineno, type(left), tokens[start].line, specifier);
            exit(0);
        }
    } else {
        if (!strcmp(type(left), "int")) {
            if (strcmp(type(right), "int")) {
                char specifier[MAX_LINE_LEN] = "";
                repeat_char(' ', tokens[ind - 1].col + strlen(tokens[ind - 1].value), specifier);
                strncat(specifier, "^", 2);
                printf("On line %d:\nExpected integer on right side of expression\n%s\n%s\n", tokens[ind - 1].lineno, tokens[ind - 1].line, specifier);
                exit(0);
            }
        }
        else if (!strcmp(type(left), "real")) {
            if (strcmp(type(right), "real")) {
                char specifier[MAX_LINE_LEN] = "";
                repeat_char(' ', tokens[ind - 1].col, specifier);
                strncat(specifier, "^", 2);
                printf("On line %d:\nExpected real on right side of expression\n%s\n%s\n", tokens[ind - 1].lineno, tokens[ind - 1].line, specifier);
                exit(0);
            }
        } else {
            char specifier[MAX_LINE_LEN] = "";
            repeat_char(' ', tokens[start].col, specifier);
            strncat(specifier, "^", 2);
            printf("On line %d:\nInvalid type `%s` on left side of expression\n%s\n%s\n", tokens[start].lineno, type(left), tokens[start].line, specifier);
            exit(0);
        }
    }
}

Node* expression2(int start) {
    ind = start;
    Node* t = term(start);
    if (t == NULL) {
        ind = start;
        return NULL;
    }
    char* oper = expect_type(T_PLUS);
    if (oper == NULL) {
        oper = expect_type(T_MINUS);
        if (oper == NULL) {
            ind = start;
            free_node(t);
            return NULL;
        }
    }
    Node* expr = expression(ind);
    if (expr == NULL) {
        char specifier[100] = "";
        repeat_char(' ', tokens[ind - 1].col + strlen(tokens[ind - 1].value), specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nExpected right hand side of expression\n%s\n%s\n", tokens[ind - 1].lineno, tokens[ind - 1].line, specifier);
        exit(0);
    }
    check_type(start, t, expr, oper);
    return (Node*) new_Operator_node(oper, t, expr);
}

Node* expression(int start) {
    ind = start;
    Node* expr2 = expression2(start);
    if (expr2 != NULL) {
        return expr2;
    }
    Node* t = term(start);
    if (t != NULL) {
        free_node(expr2);
        return t;
    }
    ind = start;
    return NULL;
}

Node* factor1(int start) {
    ind = start;
    char* left = expect_type(T_LEFT_PAREN);
    if (left == NULL) {
        ind = start;
        return NULL;
    }
    Node* expr = expression(ind);
    if (expr == NULL) {
        ind = start;
        /* TODO: Give error */
        return NULL;
    }
    char* right = expect_type(T_RIGHT_PAREN);
    if (right == NULL) {
        ind = start;
        /* TODO: Give error */
        return NULL;
    }
    return expr;
}

Node* factor(int start) {
    ind = start;
    Node* f1 = factor1(start);
    if (f1 != NULL) {
        return f1;
    }
    char* integer = expect_type(T_INT);
    if (integer != NULL) {
        return (Node*) new_Integer_node(atoi(integer));
    }
    char* string = expect_type(T_STR);
    if (string != NULL) {
        return (Node*) new_String_node(string);
    }
    char* ch = expect_type(T_CHAR);
    if (ch != NULL) {
        return (Node*) new_Char_node(*ch);
    }
    char* real = expect_type(T_REAL);
    if (real != NULL) {
        return (Node*) new_Real_node(strtod(real, NULL));
    }
    Node* func_call = incomplete_function_call(ind);
    if (func_call != NULL) {
        return func_call;
    }
    char* id = expect_type(T_ID);
    if (id != NULL) {
        return (Node*) new_Identifier_node(id, symtab_find_global(id, "var")->cgid, symtab_find_global(id, "var")->type);
    }
    ind = start;
    return NULL;
}

Node* term2(int start) {
    ind = start;
    Node* f = factor(start);
    if (f == NULL) {
        ind = start;
        return NULL;
    }
    char* oper = expect_type(T_TIMES);
    if (oper == NULL) {
        oper = expect_type(T_DIVIDE);
        if (oper == NULL) {
            ind = start;
            free_node(f);
            return NULL;
        }
    }
    Node* t = term(ind);
    if (t == NULL) {
        char specifier[100] = "";
        repeat_char(' ', tokens[ind - 1].col + strlen(tokens[ind - 1].value), specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nExpected right hand side of expression\n%s\n%s\n", tokens[ind - 1].lineno, tokens[ind - 1].line, specifier);
        exit(0);
    }
    if (strcmp(oper, "/") == 0 && t->n_type == INTEGER_NODE) {
        if (((Integer_node*) t)->value == 0) {
            /* TODO error */
            printf("can't divide by zero\n");
            exit(0);
        }
    }
    check_type(start, f, t, oper);
    return (Node*) new_Operator_node(oper, f, t);
}

Node* term(int start) {
    ind = start;
    Node* t2 = term2(start);
    if (t2 != NULL) {
        return t2;
    }
    Node* f = factor(start);
    if (f != NULL) {
        return f;
    }
    ind = start;
    return NULL;
}

// end expressions parsing
// begin statement parsing

void func_expr_args(int start, Node** args, int* len) {
    ind = start;
    int arg_c = 0;
    int should_find = 0;
    while (1) {
        Node* expr = expression(ind);
        log_trace("typeofexpr: |%s|\n", type(expr));
        if (expr == NULL) {
            if (should_find) {
                char specifier[1024] = {'\0'};
                repeat_char(' ', tokens[ind - 1].col + strlen(tokens[ind - 1].value), specifier);
                strncat(specifier, "^", 2);
                printf("On line %d:\nExpected argument\n%s\n%s\n", tokens[ind - 1].lineno, tokens[ind - 1].line, specifier);
                exit(0);
            } else {
                break;
            }
        }
        char* comma = expect_type(T_COMMA);
        args[arg_c++] = expr;
        if (comma == NULL) {
            break;
        }
        should_find = 1;
    }
    *len = arg_c;
}

Node* incomplete_function_call(int start) {
    ind = start;
    char* id = expect_type(T_ID);
    if (id == NULL) {
        ind = start;
        return NULL;
    }
    char* left = expect_type(T_LEFT_PAREN);
    if (left == NULL) {
        ind = start;
        return NULL;
    }
    Node* args[1024];
    memset(args, 0, 1024);
    int args_len;
    func_expr_args(ind, args, &args_len);
    char b[MAX_NAME_LEN];
    consume(T_RIGHT_PAREN, "Expected closing parenthesis\n", b);
    if (!symtab_find_global(id, "func")) {
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[start + 1].col, specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nUndefined function `%s`\n%s\n%s\n", tokens[start + 1].lineno, id, tokens[start + 1].line, specifier);
        exit(0);
    }
    if (args_len != symtab_find_global(id, "func")->args_len) {
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[start + 1].col, specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nWrong amount of arguments for function `%s`\n%s\n%s\n", tokens[start + 1].lineno, id, tokens[start + 1].line, specifier);
        exit(0);
    }
    return (Node*) new_Func_call_node(id, args);
}

Node* incomplete_initializers(char* t) {
    if (strcmp(t, "int") == 0) {
        return (Node*) new_Integer_node(0);
    } else if (strcmp(t, "real") == 0) {
        return (Node*) new_Real_node(0.0);
    } else if (strcmp(t, "string") == 0) {
        return (Node*) new_String_node("\\00");
    }
    return (Node*) new_Integer_node(0);
}

Node* incomplete_var_declaration(int start) {
    ind = start;
    char* type = expect_type(T_TYPE);
    if (type == NULL) {
        ind = start;
        return NULL;
    }
    char* id = expect_type(T_ID);
    if (id == NULL) {
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[ind - 1].col + strlen(tokens[ind - 1].value), specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nExpected identifier after type\n%s\n%s\n", tokens[ind - 1].lineno, tokens[ind - 1].line, specifier);
        exit(0);
    }
    char* end = expect_type(T_SEMI_COLON);
    if (end == NULL) {
        end = expect_type(T_ASSIGN);
        if (end == NULL) {
            ind = start;
            return NULL;
        } else {
            ind = start;
            return NULL;
        }
    }
    if (symtab_find_local(id, "var") != NULL) {
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[start + 1].col, specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nRedeclaration of variable `%s`\n%s\n%s\n", tokens[start + 1].lineno, tokens[start + 1].value, tokens[start + 1].line, specifier);
        exit(0);
    }
    char cgid[MAX_NAME_LEN + 4] = {0};
    snprintf(cgid, MAX_NAME_LEN + 4, "%s.%d", id, id_c++);
    symtab_add_symbol(type, "var", id, 0, cgid);
    return (Node*) new_Var_declaration_node(type, cgid, id, incomplete_initializers(type));
}

Node* function_call(int start) {
    ind = start;
    char* id = expect_type(T_ID);
    if (id == NULL) {
        ind = start;
        return NULL;
    }
    char b[MAX_NAME_LEN];
    consume(T_LEFT_PAREN, "Expected opening parenthesis after identifier\n", b);
    int args_len;
    Node* args[1024];
    memset(args, 0, sizeof(args));
    func_expr_args(ind, args, &args_len);
    /*for (int i = 0; i < args_len; i++) {
        print_node(stdout, args[i]);
    }*/
    char b2[MAX_NAME_LEN];
    consume(T_RIGHT_PAREN, "Expected closing parenthesis after arguments\n", b2);
    char b3[MAX_NAME_LEN];
    consume(T_SEMI_COLON, "Expected semi-colon to complete statement\n", b3);
    if (symtab_find_global(id, "func") == NULL) {
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[start + 1].col, specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nUndefined function `%s`\n%s\n%s\n", tokens[start + 1].lineno, id, tokens[start + 1].line, specifier);
        exit(0);
    }
    if (args_len != symtab_find_global(id, "func")->args_len) {
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[start + 1].col, specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nWrong amount of arguments for function `%s`\n%s\n%s\n", tokens[start + 1].lineno, id, tokens[start + 1].line, specifier);
        exit(0);
    }
    return (Node*) new_Func_call_node(id, args);
}

Node* var_declaration(int start) {
    ind = start;
    char* var_type = expect_type(T_TYPE);
    if (var_type == NULL) {
        ind = start;
        return NULL;
    }
    char* id = expect_type(T_ID);
    if (id == NULL) {
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[ind - 1].col + strlen(tokens[ind - 1].value), specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nExpected identifier after type\n%s\n%s\n", tokens[ind - 1].lineno, tokens[ind - 1].line, specifier);
        exit(0);
    }
    char b[MAX_NAME_LEN];
    consume(T_ASSIGN, "Expected assignment operator, opening parenthesis or semi-colon after type and identifier\n", b);
    Node* expr = expression(ind);
    if (expr == NULL) {
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[ind - 1].col + strlen(tokens[ind - 1].value), specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nExpected expression after assignment operator\n%s\n%s\n", tokens[ind - 1].lineno, tokens[ind - 1].line, specifier);
        exit(0);
    }
    char b2[MAX_NAME_LEN];
    consume(T_SEMI_COLON, "Expected semi-colon to complete statement\n", b2);
    if (symtab_find_local(id, "var") != NULL) {
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[start + 1].col, specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nRedeclaration of variable `%s`\n%s\n%s\n", tokens[start + 1].lineno, tokens[start + 1].value, tokens[start + 1].line, specifier);
        exit(0);
    }
    if (strcmp(type(expr), var_type)) {
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[start + 1].col, specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nFor variable `%s`\nAssignment to type %s from different type %s\n%s\n%s\n", tokens[start + 1].lineno, tokens[start + 1].value, var_type, type(expr), tokens[start + 1].line, specifier);
        exit(0);
    }
    char cgid[MAX_NAME_LEN + 4] = {0};
    snprintf(cgid, MAX_NAME_LEN + 4, "%s.%d", id, id_c++);
    symtab_add_symbol(var_type, "var", id, 0, cgid);
    return (Node*) new_Var_declaration_node(var_type, cgid, id, expr);
}

void program(Node** ast, int max_len);
void parse(Token* toks, Node** program, Symbol** sym_t);

Node* block_statement(int start) {
    ind = start;
    char* begin = expect_type(T_LEFT_BRACE);
    if (begin == NULL) {
        ind = start;
        return NULL;
    }
    int found_end = 0;
    Node* statements[1024];
    memset(statements, 0, sizeof(statements));
    Token block_tokens[1024];
    memset(block_tokens, 0, sizeof(block_tokens));
    int count = 0;
    int i = 0;
    printf("ind %d tokslen %d", ind, tokslen(tokens));
    for (count = ind; count < tokslen(tokens); count++) {
        if (tokens[count].type == T_RIGHT_BRACE) {
            count++;
            found_end = 1;
            break;
        }
        block_tokens[i++] = tokens[count];
    }
    printf("ind %d tokslen %d", ind, tokslen(tokens));
    symtab_push_context();
    program(statements, count);
    symtab_pop_context();
    int size = 0;
    for (size = 0; size < sizeof(statements) / sizeof(Node*); size++) {
        if (statements[size] == NULL) {
            break;
        }
    }
    log_trace("size is: %d\n", size);
    if (found_end) {
        return (Node*) new_Block_node(statements, size);
    } else { 
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[start + 1].col, specifier);
        strncat(specifier, "^", 2);
        printf("On line %d:\nExpected closing bracket\n%s\n%s\n", tokens[start + 1].lineno, tokens[start + 1].line, specifier);
        exit(0);
    }
}

Node* statement(int start) {
    ind = start;
    Node* i_var = incomplete_var_declaration(start);
    if (i_var != NULL) {
        return i_var;
    }
    Node* var = var_declaration(start);
    if (var != NULL) {
        return var;
    }
    Node* func = function_call(start);
    if (func != NULL) {
        return func;
    }
    Node* block = block_statement(start);
    if (block != NULL) {
        log_trace("found block statement\n");
        return block;
    }
    ind = start;
    return NULL;
}

// end statement parsing

void program(Node** ast, int max_len) {
    if (max_len == -1) {
        max_len = tokslen(tokens);
    }
    int stmt_c = 0;
    while (ind < max_len) {
        Node* stmt = statement(ind);
        if (stmt == NULL) {
            log_trace("stmt is NULL %d\n", stmt->n_type);
            break;
        }
        log_trace("stmt address %p\n", stmt);
        ast[stmt_c++] = stmt;
    }
}

void parse(Token* toks, Node** ast, Symbol** sym_t) {
    symtab_init();
    symtab_add_symbol("none", "var", "none", 0, "none");
    symtab_add_symbol("none", "func", "write", 1, "write");
    symtab_add_symbol("string", "func", "read", 0, "read");
    for (int i = 0; i < tokslen(toks); i++) {
        tokens[i] = toks[i];
    }
    program(ast, -1);
    symtab_destroy();
    return;
}
