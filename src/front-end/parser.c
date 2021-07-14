#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#include "../common/include/log.h"
#include "scanner.h"
#include "symbols.h"
#include "../back-end/heap.h"
#include "../back-end/codegen.h"

int id_c = 0;
int nested_loops = 0;
char* function_type;
char* current_loop_begin;
char* global_code;
char* current_loop_end;
int in_function = 0;
int has_returned_in_definite_scope = 0;
int ind = 0;
Token* tokens;

// begin helper functions

void Error(Token token, const char* error, const int after) { /* Gives errors */
    fprintf(stderr, "\x1b[31;1merror\x1b[0m: On line %d\n%s\n%s\n", token.lineno, error, token.line);
    int col = after ? token.col + 1 : token.col;
    for (int i = 0; i < col; i++) {
        fprintf(stderr, " ");
    }
    if (after || strlen(token.value) == 1) {
        for (int i = 1; i < strlen(token.value); i++) {
            fprintf(stderr, " ");
        }
        fprintf(stderr, "\x1b[32;1m^\x1b[0m");
    }
    if (strlen(token.value) > 1 && !after) {
        for (int i = 1; i < strlen(token.value); i++) {
            fprintf(stderr, "\x1b[32;1m^\x1b[0m");
        }
        fprintf(stderr, "\x1b[32;1m^\x1b[0m");
    }
    fprintf(stderr, "\n");
    exit(-1);
}

int tokslen(Token* tokens) { /* Returns the length of a tokens array */
    int len = 0;
    for (int i = 0;; i++) {
        if (tokens[i].type < 200 || tokens[i].type > 247) {
            break;
        }
        len++;
    }
    return len;
}

void consume(TokenType type, char* err) { /* Consumes a token and gives error if not right token */
    if (ind >= tokslen(tokens)) {
        Error(tokens[ind - 1], err, 1);
    }
    if (tokens[ind].type == type) {
        ind++;
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

void gizmo_type(int start, char* end_type) {
    char* reg_type = expect_type(T_TYPE);
    if (reg_type == NULL) {
        ind = start;
        strcpy(end_type, "!");
        return;
    }
    strcpy(end_type, reg_type);
    while (expect_type(T_LEFT_BRACKET) != NULL) {
        char* right = expect_type(T_RIGHT_BRACKET);
        if (right == NULL) {
            Error(tokens[ind - 1], "Expected right bracket after left bracket", 1);
        }
        strcat(end_type, "[]");
    }
}

Node* expression(int start);
Node* logical(int start);
Node* term(int start);
Node* equality(int start);
Node* comparison(int start);
Node* factor(int start);
Node* primary(int start);
Node* indexed(int start);
Node* unary(int start);

// begin expressions parsing

Node* incomplete_function_call(int start);

int is_logical_operator(char* operator) {
    return (strcmp(operator, "<") == 0) || (strcmp(operator, ">") == 0) || (strcmp(operator, "<=") == 0) || (strcmp(operator, ">=") == 0) || (strcmp(operator, "==") == 0) || (strcmp(operator, "!=") == 0);
}

void get_type_from_str(char* str, char* end) {
    for (int i = 0; i < strlen(str) - 2; i++) {
        char c = str[i];
        strncat(end, &c, 1);
    }
}

char* type(Node* n) { /* Returns the type of the given Node* */
    if (n == NULL) {
        return NULL;
    }
    char* str;
    switch (n->n_type) {
        case OPERATOR_NODE:
            if (((Operator_node*) n)->left->n_type == INTEGER_NODE && ((Operator_node*) n)->right->n_type == INTEGER_NODE && *((Operator_node*) n)->oper == '/') {
                if (((Integer_node*)((Operator_node*) n)->right)->value == 0) {
                    fprintf(stderr, "Can't divide by zero!\n");
                    exit(1);
                }
                if (((Integer_node*) ((Operator_node*) n)->left)->value % ((Integer_node*) ((Operator_node*) n)->right)->value == 0) {
                    return "int";
                }
                return "real";
            }
            if (is_logical_operator(((Operator_node*) n)->oper)) {
                return "bool";
            }
            return type(((Operator_node*) n)->left);
        case INTEGER_NODE:
            return "int";
        case LIST_NODE:
            return ((List_node*) n)->type;
        case INDEX_NODE:
            str = heap_alloc(100);
            memset(str, 0, 100);
            if (strcmp(type(((Index_node*) n)->id), "string") == 0) {
                strcpy(str, "char");
            } else {
                get_type_from_str(type(((Index_node*) n)->id), str);
            }
            return str;
        case BOOL_NODE:
            return "bool";
        case BLOCK_NODE:
            break;
        case CHAR_NODE:
            return "char";
        case STRING_NODE:
            return "string";
        case REAL_NODE:
            return "real";
        case VAR_DECLARATION_NODE:
        case VAR_ASSIGN_NODE:
            break;
        case ID_NODE:
            return ((Identifier_node*) n)->type;
        case FUNC_CALL_NODE:
        case READ_NODE:
        case WRITE_NODE:
        case LEN_NODE:
           return symtab_find_global(((Func_call_node*) n)->name, "func")->type;
        case FUNC_DECL_NODE:
        case IF_NODE:
        case WHILE_NODE:
        case RET_NODE:
        case NODE_NODE:
        case SKIP_NODE:
            break;
    }
    return NULL;
}

void check_type(int start, Node* left, Node* right, char* oper) { /* Checks if expressions follow the type rules */
    if (strcmp(oper, "+") == 0) {
        if (!strcmp(type(left), "int")) {
            if (strcmp(type(right), "int")) {
                Error(tokens[start + 2], "Expected integer on right side of expression", 0);
            }
        }
        else if (!strcmp(type(left), "string")) {
            if (strcmp(type(right), "string")) {
                Error(tokens[ind - 1], "Expected string on right side of expression", 0);
            }
        }
        else if (!strcmp(type(left), "real")) {
            if (strcmp(type(right), "real")) {
                Error(tokens[start + 2], "Expected real on right side of expression", 0);
            }
        } else {
            Error(tokens[start], str_format("Invalid operand type '%s' for operator '+'", type(left)), 0);
        }
    } else if (strcmp(oper, "and") == 0 || strcmp(oper, "or") == 0) {
        if (strcmp(type(left), "bool") == 0) {
            if (strcmp(type(right), "bool")) {
                Error(tokens[start + 2], "Expected boolean on right side of expression", 0);
            }
        } else {
            Error(tokens[start], str_format("Invalid operand type '%s' for logical operator '%s'", type(left), oper), 0);
        }
    } else if (strcmp(oper, "%") == 0) {
        if (strcmp(type(left), "int") == 0) {
            if (strcmp(type(right), "int")) {
                Error(tokens[start + 2], "Expected integer on right side of expression", 0);
            }
        } else {
            Error(tokens[start], str_format("Invalid operand type '%s' for operator `%s`", type(left)), 0);
        }
    } else if (strcmp(oper, "==") == 0 || strcmp(oper, "!=") == 0) {
        if (!strcmp(type(left), "int")) {
            if (strcmp(type(right), "int")) {
                Error(tokens[start + 2], "Expected integer on right side of expression", 0);
            }
        }
        else if (!strcmp(type(left), "string")) {
            if (strcmp(type(right), "string")) {
                Error(tokens[ind - 1], "Expected string on right side of expression", 0);
            }
        }
        else if (!strcmp(type(left), "real")) {
            if (strcmp(type(right), "real")) {
                Error(tokens[start + 2], "Expected real on right side of expression", 0);
            }
        } else if (!strcmp(type(left), "bool")) {
            if (strcmp(type(right), "bool")) {
                Error(tokens[start + 2], "Expected boolean on right side of expression", 0);
            }
        } else if (!strcmp(type(left), "char")) {
            if (strcmp(type(right), "char")) {
                Error(tokens[start + 2], "Expected character on right side of expression", 0);
            }
        } else {
            Error(tokens[start], str_format("Invalid operand type '%s' for operator '%s'", type(left), oper), 0);
        }
    } else if (is_logical_operator(oper)) {
        if (!strcmp(type(left), "int")) {
            if (strcmp(type(right), "int")) {
                Error(tokens[start + 2], "Expected integer on right side of expression", 0);
            }
        } else if (!strcmp(type(left), "real")) {
            if (strcmp(type(right), "real")) {
                Error(tokens[start + 2], "Expected real on right side of expression", 0);
            }
        } else if (!strcmp(type(left), "char")) {
            if (strcmp(type(right), "char")) {
                Error(tokens[start + 2], "Expected character on right side of expression", 0);
            }
        } else {
            Error(tokens[start], str_format("Invalid operand type '%s' for operator '%s'", type(left), oper), 0);
        }
    } else {
        if (!strcmp(type(left), "int")) {
            if (strcmp(type(right), "int")) {
                Error(tokens[start + 2], "Expected integer on right side of expression", 0);
            }
        }
        else if (!strcmp(type(left), "real")) {
            if (strcmp(type(right), "real")) {
                Error(tokens[start + 2], "Expected real on right side of expression", 0);
            }
        } else {
            Error(tokens[start], str_format("Invalid operand type '%s' for operator '%s'", type(left), oper), 0);
        }
    }
}

Node* expression(int start) {
    return logical(start);
}

Node* logical(int start) {
    ind = start;
    Node* expr = equality(ind);

    while (expect_type(T_AND) != NULL || expect_type(T_OR) != NULL) {
        if (expr == NULL) {
            Error(tokens[ind - 1], "Unexpected token", 0);
        }
        int save = ind - 1;
        Node* right = equality(ind);
        if (right == NULL) {
            Error(tokens[save], "Expected right side of expression", 1);
        }
        check_type(start, expr, right, tokens[save].value);
        expr = (Node*) new_Operator_node(tokens[save].value, expr, right);
    }

    return expr;
}

Node* equality(int start) {
    ind = start;
    Node* expr = comparison(ind);

    while (expect_type(T_NOT_EQUALS) != NULL || expect_type(T_EQUALS_EQUALS) != NULL) {
        if (expr == NULL) {
            Error(tokens[ind - 1], "Unexpected token", 0);
        }
        int save = ind - 1;
        Node* right = comparison(ind);
        if (right == NULL) {
            Error(tokens[save], "Expected right side of expression", 1);
        }
        check_type(start, expr, right, tokens[save].value);
        expr = (Node*) new_Operator_node(tokens[save].value, expr, right);
    }

    return expr;
}

Node* comparison(int start) {
    ind = start;
    Node* expr = term(ind);

    while (expect_type(T_GREATER_THAN) != NULL || expect_type(T_LESS_THAN) != NULL || expect_type(T_GREATER_THAN_EQUALS) != NULL || expect_type(T_LESS_THAN_EQUALS) != NULL) {
        if (expr == NULL) {
            Error(tokens[ind - 1], "Unexpected token", 0);
        }
        int save = ind - 1;
        Node* right = term(ind);
        if (right == NULL) {
            Error(tokens[save], "Expected right side of expression", 1);
        }
        check_type(start, expr, right, tokens[save].value);
        expr = (Node*) new_Operator_node(tokens[save].value, expr, right);
    }

    return expr;
}

Node* term(int start) {
    ind = start;
    Node* expr = factor(ind);

    while (expect_type(T_PLUS) != NULL || expect_type(T_MINUS) != NULL) {
        if (expr == NULL) {
            Error(tokens[ind - 1], "Unexpected token", 0);
        }
        int save = ind - 1;
        Node* right = factor(ind);
        if (right == NULL) {
            Error(tokens[save], "Expected right side of expression", 1);
        }
        check_type(start, expr, right, tokens[save].value);
        expr = (Node*) new_Operator_node(tokens[save].value, expr, right);
    }

    return expr;
}

Node* factor(int start) {
    ind = start;
    Node* expr = unary(ind);

    while (expect_type(T_TIMES) != NULL || expect_type(T_DIVIDE) != NULL || expect_type(T_MOD) != NULL) {
        if (expr == NULL) {
            Error(tokens[ind - 1], "Unexpected token", 0);
        }
        int save = ind - 1;
        Node* right = unary(ind);
        if (right == NULL) {
            Error(tokens[save], "Expected right side of expression", 1);
        }
        check_type(start, expr, right, tokens[save].value);
        expr = (Node*) new_Operator_node(tokens[save].value, expr, right);
    }

    return expr;
}

Node* unary(int start) {
	ind = start;

    if (expect_type(T_MINUS) != NULL || expect_type(T_NOT) != NULL) {
        int save = ind - 1;
        Node* right = unary(ind);
        if (right == NULL) {
            Error(tokens[save], "Expected something after operator", 1);
        }
        return (Node*) new_Operator_node(tokens[save].type == T_MINUS ? "*" : "!=", (Node*) new_Integer_node(tokens[save].type == T_MINUS ? -1 : 1), right);
    }

    return indexed(ind);
}

Node* indexed(int start) {
    ind = start;

    Node* expr = primary(ind);

    while (expect_type(T_LEFT_BRACKET) != NULL) {
        if (expr == NULL) {
            Error(tokens[ind - 1], "Unexpected token", 0);
        }
        int save = ind - 1;
        Node* value = expression(ind);
        if (value == NULL) {
            Error(tokens[save], "Expected expression after left bracket", 1);
        }
        expect_type(T_RIGHT_BRACKET);
        char typ[MAX_TYPE_LEN];
        strcpy(typ, "");
        if (type(expr)[strlen(type(expr)) - 1] == ']') {
            get_type_from_str(type(expr), typ);
        } else if (strcmp(type(expr), "string") == 0) {
            strcpy(typ, "char");
        } else {
            printf("In dev error: Cannot index a non-list value\n");
            exit(-1);
        }
        expr = (Node*) new_Index_node(expr, value, typ, "");
    }

    return expr;
}

void func_expr_args(int start, Node** args, int* len);

Node* primary(int start) {
	ind = start;

    if (expect_type(T_TRUE) != NULL) {
        return (Node*) new_Boolean_node(1);
    }

    if (expect_type(T_FALSE) != NULL) {
        return (Node*) new_Boolean_node(0);
    }

	if (expect_type(T_INT) != NULL) {
        return (Node*) new_Integer_node(atoi(tokens[ind - 1].value));
    }

    Node* func_call = incomplete_function_call(ind);
    if (func_call != NULL) {
        return func_call;
    }

    if (expect_type(T_ID) != NULL) {
        int begin = ind - 1;
        if (symtab_find_global(tokens[ind - 1].value, "var") == NULL) {
            Error(tokens[begin], "Use of undefined variable", 0);
        }
        return (Node*) new_Identifier_node(tokens[begin].value, symtab_find_global(tokens[begin].value, "var")->cgid, symtab_find_global(tokens[begin].value, "var")->type);
    }

    if (expect_type(T_REAL) != NULL) {
        return (Node*) new_Real_node(strtof(tokens[ind - 1].value, NULL));
    }

    if (expect_type(T_CHAR) != NULL) {
        return (Node*) new_Char_node(tokens[ind - 1].value[0]);
    }

    if (expect_type(T_STR) != NULL) {
        return (Node*) new_String_node(tokens[ind - 1].value);
    }

    if (expect_type(T_LEFT_PAREN) != NULL) {
        Node* expr = expression(ind);
        consume(T_RIGHT_PAREN, "Expect ')' after expression");
        return expr;
    }

    if (expect_type(T_LEFT_BRACKET) != NULL) {
        Node* list[1024]; // LIMIT
        memset(list, 0, 1024); // AND HERE
        int len = 0;
        int save = ind - 1;
        func_expr_args(ind, list, &len);
        char* should_be_type = type(list[0]);
        for (int i = 1; i < len; i++) {
            if (strcmp(type(list[i]), should_be_type) != 0) {
                Error(tokens[save], "Expected all array elements to be the same type", 0);
            }
        }
        consume(T_RIGHT_BRACKET, "Expect ']' after array elements");
        char list_type[MAX_TYPE_LEN];
        if (len == 0) {
            Error(tokens[save], "Cannot figure out what type this array is", 0);
        }
        strcpy(list_type, type(list[0]));
        strcat(list_type, "[]");
        return (Node*) new_List_node(list_type, list);
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
        int save = ind;
        Node* expr = expression(ind);
        if (expr == NULL) {
            if (should_find) {
                Error(tokens[ind - 1], "Expected argument after comma", 1);
            } else {
                ind = save;
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

void func_decl_args(int start, Node** args, int* len) {
    ind = start;
    int arg_c = 0;
    int should_find = 0;
    while (1) {
        int save = ind;
        char arg_type[MAX_TYPE_LEN];
        gizmo_type(ind, arg_type);
        if (strcmp(arg_type, "!") == 0) {
            if (should_find) {
                Error(tokens[ind - 1], "Expected type after comma", 1);
            } else {
                ind = save;
                break;
            }
        }
        if (strcmp(arg_type, "auto") == 0) {
            Error(tokens[ind - 1], "Cannot use auto type for arguments", 0);
        }
        char* arg_id = expect_type(T_ID);
        if (arg_id == NULL) {
            Error(tokens[ind - 1], "Expected identifier after type", 1);
        }
        char* comma = expect_type(T_COMMA);
        args[arg_c++] = (Node*) new_Var_declaration_node(arg_type, str_format(".%d", id_c++), arg_id, NULL);
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
    Node* args[1024]; // LIMIT
    memset(args, 0, 1024); // AND HERE
    int args_len;
    func_expr_args(ind, args, &args_len);
    consume(T_RIGHT_PAREN, "Expected closing parenthesis\n");
    if (!symtab_find_global(id, "func")) {
        Error(tokens[start], str_format("Use of undefined function '%s'", id), 0);
    }
    if (args_len != symtab_find_global(id, "func")->args_len) {
        Error(tokens[start], str_format("Wrong amount of arguments for function '%s'\nExpected %d, found %d", id, symtab_find_global(id, "func")->args_len, args_len), 0);
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
    char var_type[MAX_TYPE_LEN];
    gizmo_type(ind, var_type);
    if (strcmp(var_type, "!") == 0) {
        ind = start;
        return NULL;
    }
    char* id = expect_type(T_ID);
    if (id == NULL) {
        ind = start;
        return NULL;
    }
    char* end = expect_type(T_SEMI_COLON);
    if (end == NULL) {
        ind = start;
        return NULL;
    }
    if (symtab_find_local(id, "var") != NULL) {
        Error(tokens[start + 1], str_format("Redefinition of variable '%s'", id), 0);
    }
    if (strcmp(var_type, "none") == 0) {
        Error(tokens[start], "Cannot use `none` like a type", 0);
    }
    if (strcmp(var_type, "auto") == 0) {
        Error(tokens[start], "Cannot use auto type for an incomplete variable declaration", 0);
    }
    symtab_add_symbol(var_type, "var", id, 0, str_format(".%d", id_c));
    return (Node*) new_Var_declaration_node(var_type, str_format(".%d", id_c++), id, incomplete_initializers(var_type));
}

Node* function_call(int start) { /* A function call with a semi-colon */
    ind = start;
    char* id = expect_type(T_ID);
    if (id == NULL) {
        ind = start;
        return NULL;
    }
    char* open = expect_type(T_LEFT_PAREN);
    if (open == NULL) {
        ind = start;
        return NULL;
    }
    int args_len;
    Node* args[1024]; // LIMIT
    memset(args, 0, 1024); // AND HERE
    func_expr_args(ind, args, &args_len);
    /*for (int i = 0; i < args_len; i++) {
        print_node(stdout, args[i]);
    }*/
    consume(T_RIGHT_PAREN, "Expected closing parenthesis after arguments\n");
    consume(T_SEMI_COLON, "Expected semi-colon to complete statement\n");
    if (symtab_find_global(id, "func") == NULL) {
        char* error = malloc(100);
        memset(error, 0, 100);
        snprintf(error, 100, "Use of undefined function `%s`", id);
        Error(tokens[start], error, 0);
    }
    if (args_len != symtab_find_global(id, "func")->args_len) {
        char* error = malloc(100);
        memset(error, 0, 100);
        snprintf(error, 100, "Wrong amount of arguments for function `%s`", id);
        Error(tokens[start], error, 0);
    }
    return (Node*) new_Func_call_node(id, args);
} 

Node* var_declaration(int start) { /* A variable declaration with a semi-colon */
    ind = start;
    char var_type[MAX_TYPE_LEN];
    gizmo_type(ind, var_type);
    if (strcmp(var_type, "!") == 0) {
        ind = start;
        return NULL;
    }
    char* id = expect_type(T_ID);
    if (id == NULL) {
        ind = start;
        return NULL;
    }
    char* eq = expect_type(T_ASSIGN);
    if (eq == NULL) {
        ind = start;
        return NULL;
    }
    Node* expr = expression(ind);
    if (expr == NULL) {
        Error(tokens[start + 2], "Expected expression after assignment operator", 1);
    }
    consume(T_SEMI_COLON, "Expected semi-colon to complete statement\n");
    if (symtab_find_local(id, "var") != NULL) {
        char* error = malloc(100);
        memset(error, 0, 100);
        snprintf(error, 100, "Redefinition of variable `%s`", id);
        free_node(expr);
        Error(tokens[start + 1], error, 0);
    }
    if (strcmp(var_type, "none") == 0) {
        Error(tokens[start], "Cannot use `none` like a type", 0);
    }
    if (strcmp(var_type, "auto") != 0) {
        if (strcmp(type(expr), var_type) != 0) {
            char* error = malloc(500);
            memset(error, 0, 500);
            snprintf(error, 500, "For variable `%s`\nCannot assign `%s` to variable of type `%s`", id, type(expr), var_type);
            Error(tokens[start], error, 0);
        }
    } else {
        strcpy(var_type, type(expr));
    }
    symtab_add_symbol(var_type, "var", id, 0, str_format(".%d", id_c));
    return (Node*) new_Var_declaration_node(var_type, str_format(".%d", id_c++), id, expr);
}

Node* var_assignment(int start) {
    ind = start;
    char* id = expect_type(T_ID);
    if (id == NULL) {
        ind = start;
        return NULL;
    }
    char* eq = expect_type(T_ASSIGN);
    if (eq == NULL) {
        Error(tokens[start], "Expected assignment operator or opening parethesis after identifier", 1);
    }
    Node* expr = expression(ind);
    if (expr == NULL) {
        Error(tokens[ind - 1], "Expected expression", 1);
    }
    consume(T_SEMI_COLON, "Expected semi-colon to complete statement");
    if (symtab_find_global(id, "var") == NULL) {
        char error[27] = {0};
        snprintf(error, 27, "Variable '%s' not created", id);
        Error(tokens[start], error, 0);
    }
    return (Node*) new_Var_assignment_node(id, expr, symtab_find_global(id, "var")->cgid);
}

void program(Node** ast, int max_len);
void parse(char* code, Token* toks, Node** program, Symbol** sym_t);
Node* statement(int start);

Node* while_statement(int start) {
    ind = start;
    char* key = expect_type(T_WHILE);
    if (key == NULL) {
        ind = start;
        return NULL;
    }
    Node* condition = expression(ind);
    if (condition == NULL) {
        Error(tokens[start], "Expected condition after while keyword", 1);
    }
    if (strcmp(type(condition), "bool") != 0) {
        Error(tokens[start + 1], "Expected condition, not expression, after while keyword", 0);
    }
    nested_loops++;
    char* bcgid = str_format(".%d", id_c++);
    char* ecgid = str_format(".%d", id_c++);
    char* save_b = malloc(strlen(current_loop_begin) + 1);
    char* save_e = malloc(strlen(current_loop_end) + 1);
    strcpy(save_b, current_loop_begin);
    strcpy(save_e, current_loop_end);
    strcpy(current_loop_end, ecgid);
    strcpy(current_loop_begin, bcgid);
    Node* body = statement(ind);
    if (body == NULL) {
        Error(tokens[ind - 1], "Expected body", 1);
    }
    strcpy(current_loop_begin, save_b);
    strcpy(current_loop_end, save_e);
    nested_loops--;
    return (Node*) new_While_loop_node(condition, body, bcgid, ecgid);
}

Node* skip_statement(int start) {
    ind = start;
    char* key = expect_type(T_SKIP);
    if (key == NULL) {
        ind = start;
        return NULL;
    }
    consume(T_SEMI_COLON, "Expected semi-colon after 'break' or 'continue'");
    char code[100]; // LIMIT
    if (nested_loops == 0) {
        Error(tokens[start], "'break' or 'continue' statements must only be in loops", 0);
    }
    snprintf(code, 100, "br label %%%s\n", strcmp(key, "break") == 0 ? current_loop_end : current_loop_begin);
    return (Node*) new_Skip_node(strcmp(key, "break") == 0 ? 0 : 1, code);
}

Node* if_statement(int start) {
    ind = start;
    char* key = expect_type(T_IF);
    if (key == NULL) {
        ind = start;
        return NULL;
    }
    Node* condition = expression(ind);
    if (condition == NULL) {
        Error(tokens[start], "Expected condition after if keyword", 1);
    }
    if (strcmp(type(condition), "bool") != 0) {
        Error(tokens[start + 1], "Expected condition, not expression, after if keyword", 0);
    }
    Node* body = statement(ind);
    if (body == NULL) {
        Error(tokens[ind - 1], "Expected body", 1);
    }
    Node* else_body = NULL;
    if (expect_type(T_ELSE) != NULL) {
        else_body = statement(ind);
        if (else_body == NULL) {
            Error(tokens[ind - 1], "Expected body after else", 1);
        }
    }
    char bcgid[100]; // LIMITS
    snprintf(bcgid, 100, ".%d", id_c++);
    char elcgid[100];
    snprintf(elcgid, 100, ".%d", id_c++);
    char ecgid[100];
    snprintf(ecgid, 100, ".%d", id_c++);
    return (Node*) new_If_node(condition, body, else_body, bcgid, ecgid, elcgid);
}

Node* block_statement(int start, Symbol** predeclared, int len_predeclared) { /* A statement with multiple statements surrounded by curly braces inside it */
    ind = start;
    char* begin = expect_type(T_LEFT_BRACE);
    if (begin == NULL) {
        ind = start;
        return NULL;
    }
    int found_end = 0;
    Node* statements[1024]; // LIMIT
    memset(statements, 0, 1024); // AND HERE
    Token block_tokens[1024]; // LIMIT
    memset(block_tokens, 0, 1024); // AND HERE
    int count = 0;
    int i = 0;
    log_trace("ind %d tokslen %d\n", ind, tokslen(tokens));
    int nests = 0;
    for (count = ind; count < tokslen(tokens); count++) {
        if (tokens[count].type == T_RIGHT_BRACE && nests <= 0) {
            count--;
            found_end = 1;
            break;
        } else if (tokens[count].type == T_LEFT_BRACE) {
            nests++;
        } else if (tokens[count].type == T_RIGHT_BRACE) {
            nests--;
        }
        block_tokens[i++] = tokens[count];
    }
    symtab_push_context();
    for (int i = 0; i < len_predeclared; i++) {
        Symbol* symbol = predeclared[i];
        symtab_add_symbol(symbol->type, symbol->sym_type, symbol->name, symbol->args_len, symbol->cgid);
        free(predeclared[i]->name);
        free(predeclared[i]->cgid);
        free(predeclared[i]->type);
        free(predeclared[i]->sym_type);
        free(predeclared[i]);
    }
    program(statements, count);
    symtab_pop_context();
    int size = 0;
    for (size = 0; size < sizeof(statements) / sizeof(Node*); size++) {
        if (statements[size] == NULL) {
            break;
        }
        //print_node(stdout, statements[size]);
    }
    ind++;
    log_trace("size is: %d\n", size);
    if (found_end) {
        log_trace("ind %d\n", ind);
        return (Node*) new_Block_node(statements, size);
    } else {
        Error(tokens[ind - 2], "Expected closing brace", 1);
        return NULL; /* To satisfy Clang */
    }
}

Node* statement(int start);

Node* return_statement(int start) {
    ind = start;
    char* key = expect_type(T_RETURN);
    if (key == NULL) {
        ind = start;
        return NULL;
    }
    Node* expr = expression(ind);
    if (expr == NULL) {
        expr = (Node*) new_Integer_node(0);
    }
    if (!in_function) {
        Error(tokens[start], "Can't have return statement outside of function", 0);
    }
    if (strcmp(function_type, type(expr)) != 0) {
        Error(tokens[start + 1], "Return type of function is different from the return expression", 0);
    }
    consume(T_SEMI_COLON, "Expected semi-colon to terminate return statement");
    return (Node*) new_Return_node(expr);
}

Node* function_declaration(int start) {
    ind = start;
    char func_type[MAX_TYPE_LEN];
    gizmo_type(ind, func_type);
    if (strcmp(func_type, "!") == 0) {
        ind = start;
        return NULL;
    }
    if (strcmp(func_type, "auto") == 0) {
        Error(tokens[start], "Cannot use auto type for functions", 0);
    }
    char* id = expect_type(T_ID);
    if (id == NULL) {
        Error(tokens[ind - 1], "Expected identifier after type", 1);
    }
    consume(T_LEFT_PAREN, "Expected opening parenthesis after type and id");
    Node* args[1024]; // LIMIT
    memset(args, 0, 1024); // AND HERE
    int args_len = 0;
    func_decl_args(ind, args, &args_len);
    consume(T_RIGHT_PAREN, "Expected closing parenthesis after arguments");
    if (in_function) {
        Error(tokens[start], "Cannot have nested functions", 0);
    }
    in_function = 1;
    strcpy(function_type, func_type);
    Symbol* predeclared[1024]; // LIMIT
    memset(predeclared, 0, 1024); // AND HERE
    for (int i = 0; i < args_len; i++) {
        Symbol* sym = malloc(sizeof(Symbol));
        Var_declaration_node* arg = (Var_declaration_node*) args[i];
        sym->name = strdup(arg->name);
        sym->type = strdup(arg->type);
        sym->sym_type = strdup("var");
        sym->cgid = strdup(arg->codegen_name);
        sym->args_len = 0;
        predeclared[i] = sym;
    }
    symtab_add_symbol(func_type, "func", id, args_len, id);
    Node* body = block_statement(ind, predeclared, args_len);
    in_function = 0;
    if (body == NULL) {
        ind = start;
        fprintf(stderr, "Expected body");
        return NULL;
    }
    return (Node*) new_Func_decl_node(id, func_type, args, args_len, body);
}

Node* statement(int start) { /* Calls all possible statements */
    ind = start;
    Node* i_var = incomplete_var_declaration(start);
    if (i_var != NULL) {
        log_trace("found incomplete var decl\n");
        return i_var;
    }
    Node* var = var_declaration(start);
    if (var != NULL) {
        log_trace("found var decl\n");
        return var;
    }
    Node* func = function_call(start);
    if (func != NULL) {
        log_trace("found function call\n");
        return func;
    }
    Node* w = while_statement(start);
    if (w != NULL) {
        return w;
    }
    Node* skip = skip_statement(start);
    if (skip != NULL) {
        return skip;
    }
    Node* if_s = if_statement(start);
    if (if_s != NULL) {
        return if_s;
    }
    Node* va = var_assignment(start);
    if (va != NULL) {
        return va;
    }
    Node* block = block_statement(start, NULL, 0);
    if (block != NULL) {
        log_trace("found block statement\n");
        return block;
    }
    Node* func_decl = function_declaration(start);
    if (func_decl != NULL) {
        return func_decl;
    }
    Node* ret = return_statement(start);
    if (ret != NULL) {
        return ret;
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
            Error(tokens[ind], "Unexpected token", 0);
        }
        log_trace("stmt address %p\n", stmt);
        ast[stmt_c++] = stmt;
    }
}

void parse(char* code, Token* toks, Node** ast, Symbol** sym_t) { /* Calls program */
    tokens = malloc(strlen(code) * sizeof(Token));
    memset(tokens, 0, strlen(code) * sizeof(Token));
    global_code = malloc(strlen(code) + 1);
    memset(global_code, 0, strlen(code) + 1);
    strcpy(global_code, code);
    symtab_init();
    function_type = malloc(MAX_TYPE_LEN);
    memset(function_type, 0, MAX_TYPE_LEN);
    current_loop_begin = malloc(MAX_NAME_LEN);
    memset(current_loop_begin, 0, MAX_NAME_LEN);
    current_loop_end = malloc(MAX_NAME_LEN);
    memset(current_loop_end, 0, MAX_NAME_LEN);
    symtab_add_symbol("none", "func", "write", 1, "write");
    symtab_add_symbol("string", "func", "read", 1, "read");
    symtab_add_symbol("int", "func", "len", 1, "len");
    for (int i = 0; i < tokslen(toks); i++) {
        tokens[i] = toks[i];
    }
    program(ast, -1);
    return;
}
