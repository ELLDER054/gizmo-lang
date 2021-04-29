#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#include "scanner.h"

int ind = 0;

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

void consume(Token tokens[1024], TokenType type, char err[100], char buffer[150]) {
    if (ind >= tokslen(tokens)) {
        char specifier[100] = "";
        repeat_char(' ', tokens[ind - 1].col, specifier);
        strncat(specifier, "^", 1);
        printf("On line %d:\n%s%s\n%s\n", tokens[ind - 1].lineno + 1, err, tokens[ind - 1].line, specifier);
        exit(0);
        return;
    }
    if (tokens[ind].type == type) {
        strcpy(buffer, tokens[ind++].value);
        return;
    }
    char specifier[100] = "";
    repeat_char(' ', tokens[ind - 1].col, specifier);
    strncat(specifier, "^", 1);
    printf("On line %d:\n%s%s\n%s\n", tokens[ind - 1].lineno + 1, err, tokens[ind - 1].line, specifier);
    exit(0);
    return;
}

char* expect_type(TokenType type, Token tokens[1024]) {
    if (ind >= tokslen(tokens)) {
        return NULL;
    }
    if (tokens[ind].type == type) {
        return tokens[ind++].value;
    }
    return NULL;
}

TokenType expect_value(char* val, Token tokens[1024]) {
    if (ind >= tokslen(tokens)) {
        return 0;
    }
    if (!strcmp(tokens[ind].value, val)) {
        return tokens[ind++].type;
    }
    return 0;
}

Node* convert_to_node(Node* n) {
    return n;
}

Node* expression(int start, Token tokens[1024]);
Node* term(int start, Token tokens[1024]);
Node* term2(int start, Token tokens[1024]);
Node* factor(int start, Token tokens[1024]);

// begin expressions parsing
Node* expression2(int start, Token tokens[1024]) {
    ind = start;
    Node* t = term(start, tokens);
    if (t == NULL) {
        ind = start;
        return NULL;
    }
    char* plus = expect_type(T_PLUS, tokens);
    if (plus == NULL) {
        ind = start;
        return NULL;
    }
    Node* expr = expression(ind, tokens);
    if (expr == NULL) {
        char specifier[100] = "";
        repeat_char(' ', tokens[ind].col, specifier);
        strncat(specifier, "^", 1);
        printf("On line %d:\nExpected right hand side of expression\n%s\n%s\n", tokens[ind].lineno, tokens[ind].line, specifier);
        exit(0);
    }
    Operator_node n;
    n.left = *t;
    n.right = *expr;
    strcpy(n.oper, "+");
    return convert_to_node((Node*) &n);
}

Node* expression(int start, Token tokens[1024]) {
    ind = start;
    Node* expr2 = expression2(start, tokens);
    if (expr2 != NULL) {
        return convert_to_node((Node*) &expr2);
    }
    Node* t = term(start, tokens);
    if (t != NULL) {
        return convert_to_node((Node*) &t);
    }
    ind = start;
    return NULL;
}

Node* factor(int start, Token tokens[1024]) {
    char* integer = expect_type(T_INT, tokens);
    if (integer != NULL) {
        Integer_node i_node;
        i_node.value = (int) integer;
        return convert_to_node((Node*) &i_node);
    }
    ind = start;
    return NULL;
}

Node* term2(int start, Token tokens[1024]) {
    ind = start;
    Node* f = factor(start, tokens);
    if (f == NULL) {
        ind = start;
        return NULL;
    }
    char* times = expect_type(T_TIMES, tokens);
    if (times == NULL) {
        ind = start;
        return NULL;
    }
    Node* t = term(ind, tokens);
    if (t == NULL) {
        char specifier[100] = "";
        repeat_char(' ', tokens[ind].col, specifier);
        strncat(specifier, "^", 1);
        printf("On line %d:\nExpected right hand side of expression\n%s\n%s\n", tokens[ind].lineno, tokens[ind].line, specifier);
        exit(0);
    }
    Operator_node n;
    n.left = *f;
    n.right = *t;
    strcpy(n.oper, "*");
    return convert_to_node((Node*) &n);
}

Node* term(int start, Token tokens[1024]) {
    ind = start;
    Node* t2 = term2(start, tokens);
    if (t2 != NULL) {
        return convert_to_node((Node*) &t2);
    }
    Node* f = factor(start, tokens);
    if (f != NULL) {
        return convert_to_node((Node*) &f);
    }
    ind = start;
    return NULL;
}

// end expressions parsing
// begin statement parsing

Node* incomplete_var_declaration(int start, Token tokens[1024]) {
    Var_declaration_node node;
    ind = start;
    char* type = expect_type(T_TYPE, tokens);
    if (type == NULL) {
        ind = start;
        return NULL;
    }
    char* id = expect_type(T_ID, tokens);
    if (id == NULL) {
        ind = start;
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[ind].col, specifier);
        strncat(specifier, "^", 1);
        printf("On line %d:\nExpected identifier after type\n%s\n%s\n", tokens[ind].lineno, tokens[ind].line, specifier);
        return NULL;
    }
    char* end = expect_type(T_SEMI_COLON, tokens);
    if (end == NULL) {
        end = expect_type(T_ASSIGN, tokens);
        if (end == NULL) {
            ind = start;
            return NULL;
        } else {
            ind = start;
            return NULL;
        }
    }
    strcpy(node.name, id);
    node.type = type;
    node.value = NULL;
    return convert_to_node((Node*) &node);
}

Node* var_declaration(int start, Token tokens[1024]) {
    Var_declaration_node node;
    ind = start;
    char* type = expect_type(T_TYPE, tokens);
    if (type == NULL) {
        ind = start;
        return NULL;
    }
    char* id = expect_type(T_ID, tokens);
    if (id == NULL) {
        ind = start;
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[ind].col, specifier);
        strncat(specifier, "^", 1);
        printf("On line %d:\nExpected identifier after type\n%s\n%s\n", tokens[ind].lineno, tokens[ind].line, specifier);
        return NULL;
    }
    char b[1] = {'\0'};
    consume(tokens, T_ASSIGN, "Expected assignment operator, opening parenthesis or semi-colon after type and identifier\n", b);
    Node* expr = expression(ind, tokens);
    if (expr == NULL) {
        ind = start;
        char specifier[1024] = {'\0'};
        repeat_char(' ', tokens[ind].col, specifier);
        strncat(specifier, "^", 1);
        printf("On line %d:\nExpected expression after assignment operator\n%s\n%s\n", tokens[ind].lineno, tokens[ind].line, specifier);
        return NULL;
    }
    consume(tokens, T_SEMI_COLON, "Expected semi-colon to complete statement\n", b);
    strcpy(node.name, id);
    node.type = type;
    return convert_to_node((Node*) &node);
}

Node* statement(int start, Token tokens[1024]) {
    ind = start;
    Node* i_var = incomplete_var_declaration(start, tokens);
    if (i_var != NULL) {
        return i_var;
    }
    Node* var = var_declaration(start, tokens);
    if (var != NULL) {
        return var;
    }
    ind = start;
    return NULL;
}

// end statement parsing

void parse(Token tokens[1024], Node program[1024]) {
    int stmt_c = 0;
    int len = tokslen(tokens);
    while (ind < len) {
        Node* stmt = statement(ind, tokens);
        if (stmt == NULL) {
            break;
        }
        program[stmt_c] = *stmt;
        printf("success!\n");
    }
    return;
}
