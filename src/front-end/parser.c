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

// begin helper functions

void Error(Token token, const char* error, const int after) { /* Gives errors */
    fprintf(stderr, "\x1b[31;1merror\x1b[0m: On line %d\n%s\n%s\n", token.lineno, error, token.line);
    int col = after ? token.col + 1 : token.col;
    for (int i = 0; i < col; i++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "\x1b[32;1m^\x1b[0m");
    if (after) {
        for (int i = 1; i < strlen(token.value); i++) {
            fprintf(stderr, " ");
        }
        fprintf(stderr, "\x1b[32;1m^\x1b[0m");
    }
    if (strlen(token.value) > 1 && !after) {
        for (int i = 1; i < strlen(token.value) - 1; i++) {
            fprintf(stderr, "\x1b[32;1m^\x1b[0m");
        }
        fprintf(stderr, "\x1b[32;1m^\x1b[0m");
    }
    fprintf(stderr, "\n");
    exit(-1);
}

int tokslen(Token* tokens) { /* Returns the length of a tokens array */
    int len = 0;
    for (int i = 0; i < 1024; i++) {
        if (tokens[i].type < 200 || tokens[i].type > 236) {
            break;
        }
        len++;
    }
    return len;
}

void consume(TokenType type, char* err, char* buffer) { /* Consumes a token and gives error if not right token */
    if (ind >= tokslen(tokens)) {
        Error(tokens[ind - 1], err, 1);
    }
    if (tokens[ind].type == type) {
        strncpy(buffer, tokens[ind++].value, MAX_NAME_LEN);
        return;
    }
    Error(tokens[ind], err, 0);
}

char* expect_type(TokenType type) { /* Expects a Token Type and if wrong type, return NULL */
    if (ind >= tokslen(tokens)) {
        return NULL;
    }
    if (tokens[ind].type == type) {
        return tokens[ind++].value;
    }
    return NULL;
}

// end helper functions

Node* expression(int start);
Node* term(int start);
Node* term2(int start);
Node* factor(int start);

// begin expressions parsing

Node* incomplete_function_call(int start);

char* type(Node* n) { /* Returns the type of the given Node* */
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
        case FUNC_DECL_NODE:
        case NODE_NODE:
            break;
    }
    return NULL;
}

void check_type(int start, Node* left, Node* right, char* oper) { /* Checks if expressions follow the type rules */
    if (*oper == '+') {
        if (!strcmp(type(left), "int")) {
            if (strcmp(type(right), "int")) {
                Error(tokens[start], "Expected integer on right side of exression", 0);
            }
        }
        else if (!strcmp(type(left), "string")) {
            if (strcmp(type(right), "string")) {
                Error(tokens[start], "Expected string on right side of expression", 0);
            }
        }
        else if (!strcmp(type(left), "real")) {
            if (strcmp(type(right), "real")) {
                Error(tokens[start], "Expected real on right side of expression", 0);
            }
        } else {
            char* error = malloc(100);
            memset(error, 0, 100);
            snprintf(error, 100, "Invalid type `%s` on left side of expression", type(left));
            Error(tokens[start], error, 0);
        }
    } else {
        if (!strcmp(type(left), "int")) {
            if (strcmp(type(right), "int")) {
                Error(tokens[start], "Expected integer on right side of expression", 0);
            }
        }
        else if (!strcmp(type(left), "real")) {
            if (strcmp(type(right), "real")) {
                Error(tokens[start], "Expected real on right side of expression", 0);
            }
        } else {
            char* error = malloc(100);
            memset(error, 0, 100);
            snprintf(error, 100, "Invalid type `%s` on left side of expression", type(left));
            Error(tokens[start], error, 0);
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
        Error(tokens[ind - 1], "Expected right side of expression", 0);
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
        if (symtab_find_global(id, "var") == NULL) {
            char* error = malloc(100);
            memset(error, 0, 100);
            snprintf(error, 100, "Use of undefined identifier `%s`", id);
            Error(tokens[ind - 1], error, 0);
            free(error);
        }
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
        Error(tokens[ind - 1], "Expected right side of expression", 0);
    }
    if (strcmp(oper, "/") == 0 && t->n_type == INTEGER_NODE) {
        if (((Integer_node*) t)->value == 0) {
            Error(tokens[ind - 1], "Can't divide by zero", 0);
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
        free_node(t2);
        return f;
    }
    ind = start;
    return NULL;
}

// end expressions parsing
// begin statement parsing

void func_expr_args(int start, Node** args, int* len) { /* Puts caller arguments in the Node** args */
    ind = start;
    int arg_c = 0;
    int should_find = 0;
    while (1) {
        Node* expr = expression(ind);
        log_trace("typeofexpr: |%s|\n", type(expr));
        if (expr == NULL) {
            if (should_find) {
                Error(tokens[ind - 1], "Expected argument", 0);
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

Node* incomplete_function_call(int start) { /* A function call with no semi-colon */
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
        char* error = malloc(100);
        memset(error, 0, 100);
        snprintf(error, 100, "Use of undefined function `%s`", id);
        Error(tokens[ind - 1], error, 0);
    }
    if (args_len != symtab_find_global(id, "func")->args_len) {
        char* error = malloc(100);
        memset(error, 0, 100);
        snprintf(error, 100, "Wrong amount of arguments for function `%s`", id);
        Error(tokens[ind - 1], error, 0);
    }
    return (Node*) new_Func_call_node(id, args);
}

Node* incomplete_initializers(char* t) { /* Returns the most low-level value for each type */
    if (strcmp(t, "int") == 0) {
        return (Node*) new_Integer_node(0);
    } else if (strcmp(t, "real") == 0) {
        return (Node*) new_Real_node(0.0);
    } else if (strcmp(t, "char") == 0) {
        return (Node*) new_Char_node(' ');
    } else if (strcmp(t, "string") == 0) {
        return (Node*) new_String_node("\\00");
    }
    return (Node*) new_Integer_node(0);
}

Node* incomplete_var_declaration(int start) { /* A variable declaration with no semi-colon */
    ind = start;
    char* type = expect_type(T_TYPE);
    if (type == NULL) {
        ind = start;
        return NULL;
    }
    char* id = expect_type(T_ID);
    if (id == NULL) {
        Error(tokens[ind], "Expected identifier after type", 0);
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
        char* error = malloc(100);
        memset(error, 0, 100);
        snprintf(error, 100, "Redefinition of variable `%s`", id);
        Error(tokens[start + 1], error, 0);
    }
    char cgid[MAX_NAME_LEN + 4] = {0};
    snprintf(cgid, MAX_NAME_LEN + 4, "%s.%d", id, id_c++);
    symtab_add_symbol(type, "var", id, 0, cgid);
    return (Node*) new_Var_declaration_node(type, cgid, id, incomplete_initializers(type));
}

Node* function_call(int start) { /* A function call with a semi-colon */
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
        char* error = malloc(100);
        memset(error, 0, 100);
        snprintf(error, 100, "Use of undefined function `%s`", id);
        Error(tokens[ind], error, 0);
    }
    if (args_len != symtab_find_global(id, "func")->args_len) {
        char* error = malloc(100);
        memset(error, 0, 100);
        snprintf(error, 100, "Wrong amount of arguments for function `%s`", id);
        Error(tokens[ind], error, 0);
    }
    return (Node*) new_Func_call_node(id, args);
}

Node* var_declaration(int start) { /* A variable declaration with a semi-colon */
    ind = start;
    char* var_type = expect_type(T_TYPE);
    if (var_type == NULL) {
        ind = start;
        return NULL;
    }
    char* id = expect_type(T_ID);
    if (id == NULL) {
       Error(tokens[ind], "Expected identifier after type", 0);
    }
    char* eq = expect_type(T_ASSIGN);
    if (eq == NULL) {
        ind = start;
        return NULL;
    }
    Node* expr = expression(ind);
    if (expr == NULL) {
        Error(tokens[ind], "Expected expression after assignment operator", 0);
    }
    char b[MAX_NAME_LEN];
    consume(T_SEMI_COLON, "Expected semi-colon to complete statement\n", b);
    if (symtab_find_local(id, "var") != NULL) {
        char* error = malloc(100);
        memset(error, 0, 100);
        snprintf(error, 100, "Redefinition of variable `%s`", id);
        free_node(expr);
        Error(tokens[start + 1], error, 0);
    }
    if (strcmp(type(expr), var_type)) {
        char* error = malloc(100);
        memset(error, 0, 100);
        snprintf(error, 100, "For variable `%s`\nAssignment to type %s from %s", id, type(expr), var_type);
        free_node(expr);
        Error(tokens[ind], error, 0);
    }
    char cgid[MAX_NAME_LEN + 4] = {0};
    snprintf(cgid, MAX_NAME_LEN + 4, "%s.%d", id, id_c++);
    symtab_add_symbol(var_type, "var", id, 0, cgid);
    return (Node*) new_Var_declaration_node(var_type, cgid, id, expr);
}

void program(Node** ast, int max_len);
void parse(Token* toks, Node** program, Symbol** sym_t);

Node* block_statement(int start) { /* A statement with multiple statements surrounded by curly braces inside it */
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
        Error(tokens[ind - 1], "Expected closing brace", 1);
        return NULL; /* To satisfy Clang */
    }
}

Node* statement(int start);

Node* function_declaration(int start) {
    ind = start;
    char* func_type = expect_type(T_TYPE);
    if (func_type == NULL) {
        ind = start;
        return NULL;
    }
    char* id = expect_type(T_ID);
    if (id == NULL) {
        ind = start; /* TODO: ERROR */
        fprintf(stderr, "Expected identifier after type");
        return NULL;
    }
    char b[100];
    consume(T_LEFT_PAREN, "Expected opening parenthesis after type and id", b);
    char b2[100];
    consume(T_RIGHT_PAREN, "Expected closing parenthesis after arguments", b2);
    Node* body = statement(ind);
    if (body == NULL) {
        ind = start;
        fprintf(stderr, "Expected body");
        return NULL;
    }
    symtab_add_symbol(func_type, "func", id, 0, id);
    return (Node*) new_Func_decl_node(id, func_type, NULL, 0, body);
}

Node* statement(int start) { /* Calls all possible statements */
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
    Node* func_decl = function_declaration(start);
    if (func_decl != NULL) {
        return func_decl;
    }
    ind = start;
    return NULL;
}

// end statement parsing

void program(Node** ast, int max_len) { /* Continuously calls statement() */
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

void parse(Token* toks, Node** ast, Symbol** sym_t) { /* Calls program */
    symtab_init();
    symtab_add_symbol("none", "var", "none", 0, "none");
    symtab_add_symbol("none", "func", "write", 1, "write");
    symtab_add_symbol("string", "func", "read", 0, "read");
    for (int i = 0; i < tokslen(toks); i++) {
        tokens[i] = toks[i];
    }
    program(ast, -1);
    return;
}
