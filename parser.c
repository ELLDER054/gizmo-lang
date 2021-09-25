#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "ast.h"

static int ind = 0;
static Token** tokens;
static int tokens_len;

static int toks_len(Token** toks) { // Returns the length of the given tokens array
    int len;
    while (toks[len] != NULL) { // Increment length until end of tokens array
        len++;
    }

    return len;
}

void consume(Token_t type, char* err) { // Consumes a token and if it finds the wrong token, give error
    if (ind >= tokens_len) {
        fprintf(stderr, "%s", err); // If there is nothing to consume, give error
    }
    if (tokens[ind]->type == type) {
        ind++; // Consume correct token if it finds it
        return;
    }
    fprintf(stderr, "%s", err); // If there is nothing to consume, give error
}

char* expect_type(Token_t type) { // Expects a Token Type and if it finds the wrong type, return NULL
    if (ind >= tokens_len) { // If there is nothing left, return NULL
        return NULL;
    }
    if (tokens[ind]->type == type) {
        return tokens[ind++]->value; // Increment and returns the token it consumed
    }
    return NULL;
}

Node* var_declaration(int start) { // Function that parses variable declarations
    ind = start;
    char* type = expect_type(TYPE_T); // Expect type (E.G. int, string, bool)
    if (type == NULL) {
        ind = start;
        return NULL;
    }
	return NULL;
}

Node* statement(int start) { // Tries all statements and returns the one that works
    Node* base_node;
    base_node = var_declaration(start);
    if (base_node != NULL) {
        return base_node;
    }
}

void program(Node** ast, int max_len) { // Continuously calls statement()
    if (max_len == -1) {
        max_len = tokens_len;
    }
    int stmt_c = 0;
    while (ind < max_len) { // Iterate through tokens
        Node* stmt = statement(ind);
        if (stmt == NULL) { // If no statements work, give error
            fprintf(stderr, "Unexpected token\n");
        }
        ast[stmt_c++] = stmt;
    }
}

void parse(Token** toks, Node** ast) { // Main parse function
    tokens_len = toks_len(toks);

    tokens = malloc(tokens_len * sizeof(Token*));
    memcpy(tokens, toks, tokens_len * sizeof(Token*));

    program(ast, -1);
}
