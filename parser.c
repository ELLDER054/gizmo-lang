#include <stdio.h>
#include <string.h>
#include "ast.h"
#include "scanner.h"

int ind = 0;

void consume(Token tokens[1024], TokenType type, char err[25], char buffer[150]) {
    if (tokens[ind].type != type) {
        printf("%s", err);
        return;
    }
    strcpy(buffer, tokens[ind].value);
}

int tokslen(Token tokens[1024]) {
    int len = 0;
    for (int i = 0; i < 1024; i++) {
        if (tokens[i].type < 200 || tokens[i].type > 240) {
            break;
        }
        len++;
    }
    return len;
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

Node* incomplete_var_declaration(int start, Token tokens[1024]) {
    Var_declaration_node node;
    ind = start;
    TokenType* type = (TokenType*) expect_type(T_TYPE, tokens);
    if (type == NULL) {
        ind = start;
        return NULL;
    }
    char* id = expect_type(T_ID, tokens);
    if (id == NULL) {
        ind = start;
        printf("Expected identifier after type\n");
        return NULL;
    }
    char* end = expect_type(T_SEMI_COLON, tokens);
    if (end == NULL) {
        ind = start;
        printf("Expected semi-colon after statement\n");
        return NULL;
    }
    strcpy(node.name, id);
    node.type = *type;
    return convert_to_node((Node*) &node);
}

Node* statement(int start, Token tokens[1024]) {
    ind = start;
    Node* var = incomplete_var_declaration(start, tokens);
    if (var != NULL) {
        return var;
    }
    ind = start;
    return NULL;
}

void parse(Token tokens[1024], Node program[1024]) {
    while (ind < tokslen(tokens)) {
        Node* stmt = statement(ind, tokens);
        if (stmt == NULL) {
            break;
        }
        printf("success!\n");
    }
    return;
}
