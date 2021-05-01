#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#include "scanner.h"

int ind = 0;
Token tokens[1024];

void repeat_char(char c, int n, char string[1024]) {
    for (int i = 0; i < n; i++) {
        strncat(string, &c, 1);
    }
}

int tokslen(Token tokens[1024]) {
    int len = 0;
    for (int i = 0; i < 1024; i++) {
        if (tokens[i].type < 200 || tokens[i].type > 236) {
            break;
        }
        len++;
    }
    return len;
}

void consume(TokenType type, char err[100], char* buffer) {
    if (ind >= tokslen(tokens)) {
        char specifier[MAX_LINE_LEN] = "";
        repeat_char(' ', tokens[ind - 1].col, specifier);
        strncat(specifier, "^", 1);
        printf("On line %d:\n%s%s\n%s\n", tokens[ind - 1].lineno + 1, err, tokens[ind - 1].line, specifier);
        exit(0);
        return;
    }
    if (tokens[ind].type == type) {
        strncpy(buffer, tokens[ind++].value, MAX_NAME_LEN);
        return;
    }
    char specifier[100] = "";
    repeat_char(' ', tokens[ind - 1].col, specifier);
    strncat(specifier, "^", 1);
    printf("On line %d:\n%s%s\n%s\n", tokens[ind - 1].lineno + 1, err, tokens[ind - 1].line, specifier);
    exit(0);
    return;
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

Node* expression(int start);
Node* term(int start);
Node* term2(int start);
Node* factor(int start);

// begin expressions parsing
Node* expression2(int start) {
    ind = start;
    Node* t = term(start);
    if (t == NULL) {
        ind = start;
        return NULL;
    }
    char* plus = expect_type(T_PLUS);
    if (plus == NULL) {
        ind = start;
        free_node(t);
        return NULL;
    }
    Node* expr = expression(ind);
    if (expr == NULL) {
        char specifier[100] = "";
        repeat_char(' ', tokens[ind].col, specifier);
        strncat(specifier, "^", 1);
        printf("On line %d:\nExpected right hand side of expression\n%s\n%s\n", tokens[ind].lineno, tokens[ind].line, specifier);
        exit(0);
    }
    return (Node*) new_Operator_node("+", t, expr);
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

Node* factor(int start) {
    char* integer = expect_type(T_INT);
    if (integer != NULL) {
        return (Node*) new_Integer_node(atoi(integer));
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
    char* times = expect_type(T_TIMES);
    if (times == NULL) {
        ind = start;
        free_node(f);
        return NULL;
    }
    Node* t = term(ind);
    if (t == NULL) {
        char specifier[100] = "";
        repeat_char(' ', tokens[ind].col, specifier);
        strncat(specifier, "^", 1);
        printf("On line %d:\nExpected right hand side of expression\n%s\n%s\n", tokens[ind].lineno, tokens[ind].line, specifier);
        exit(0);
    }
    return (Node*) new_Operator_node("*", f, t);
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

Node* incomplete_var_declaration(int start) {
    ind = start;
    char* type = expect_type(T_TYPE);
    if (type == NULL) {
        ind = start;
        return NULL;
    }
    char* id = expect_type(T_ID);
    if (id == NULL) {
        ind = start;
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[ind].col, specifier);
        strncat(specifier, "^", 1);
        printf("On line %d:\nExpected identifier after type\n%s\n%s\n", tokens[ind].lineno, tokens[ind].line, specifier);
        return NULL;
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
    return (Node*) new_Var_declaration_node(id, type, NULL);
}

Node* var_declaration(int start) {
    ind = start;
    char* type = expect_type(T_TYPE);
    if (type == NULL) {
        ind = start;
        return NULL;
    }
    char* id = expect_type(T_ID);
    if (id == NULL) {
        ind = start;
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[ind].col, specifier);
        strncat(specifier, "^", 1);
        printf("On line %d:\nExpected identifier after type\n%s\n%s\n", tokens[ind].lineno, tokens[ind].line, specifier);
        return NULL;
    }
    char b[MAX_NAME_LEN];
    consume(T_ASSIGN, "Expected assignment operator, opening parenthesis or semi-colon after type and identifier\n", b);
    Node* expr = expression(ind);
    if (expr == NULL) {
        ind = start;
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[ind].col, specifier);
        strncat(specifier, "^", 1);
        printf("On line %d:\nExpected expression after assignment operator\n%s\n%s\n", tokens[ind].lineno, tokens[ind].line, specifier);
        return NULL;
    }
    consume(T_SEMI_COLON, "Expected semi-colon to complete statement\n", b);
    return (Node*) new_Var_declaration_node(id, type, expr);
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
    ind = start;
    return NULL;
}

// end statement parsing

void parse(Token toks[1024], Node** program) {
    int stmt_c = 0;
    for (int i = 0; i < tokslen(toks); i++) {
        tokens[i] = toks[i];
    }
    int len = tokslen(tokens);
    while (ind < len) {
        Node* stmt = statement(ind);
        if (stmt == NULL) {
            break;
        }
        program[stmt_c++] = stmt;
        printf("success!\n");
    }
    return;
}
