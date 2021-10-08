#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tools.h"

static int ind = 0;
static Token** tokens;
static int tokens_len;
static int id_c = 0;

// Returns the length of the given tokens array
static int toks_len(Token** toks) {
    int len;
	// Increment length until end of tokens array
    while (toks[len] != NULL) {
        len++;
    }

    return len;
}

// Consumes a token and if it finds the wrong token, give error
void consume(Token_t type, char* err) {
    if (ind >= tokens_len) {
		// If there is nothing to consume, give error
        Error(tokens[ind - 1], err, 1);
    }
    if (tokens[ind]->type == type) {
		// Consume correct token if it finds it
        ind++;
        return;
    }
	// If there is nothing to consume, give error
    Error(tokens[ind - 1], err, 1);
}

// Expects a Token Type and if it finds the wrong type, return NULL
char* expect_type(Token_t type) {
	// If there is nothing left, return NULL
    if (ind >= tokens_len) {
        return NULL;
    }
    if (tokens[ind]->type == type) {
		// Increment index and return the tokens it found
        return tokens[ind++]->value;
    }
    return NULL;
}

char* type(Node* n) {
    if (n->n_type == INTEGER_NODE) {
        return str_format("int");
    } else if (n->n_type == STRING_NODE) {
        return str_format("string");
    } else if (n->n_type == CHAR_NODE) {
        return str_format("char");
    } else if (n->n_type == BOOL_NODE) {
        return str_format("bool");
    } else if (n->n_type == OPERATOR_NODE) {
        char* o = ((Operator_node*) n)->oper;
        if (strcmp(o, "<") == 0 || strcmp(o, ">") == 0 || strcmp(o, "<=") == 0 || strcmp(o, ">=") == 0 || strcmp(o, "!=") == 0) {
            return str_format("bool");
        } else {
            return type(((Operator_node*) n)->left);
        }
    }
    return NULL;
}

// Expression functions - Thanks to Robert Nystrom for the grammar (http://craftinginterpreters.com/parsing-expressions.html#ambiguity-and-the-parsing-game)
Node* expression(int start);
Node* logical(int start);
Node* term(int start);
Node* equality(int start);
Node* comparison(int start);
Node* factor(int start);
Node* primary(int start);
Node* indexed(int start);
Node* unary(int start);

Node* expression(int start) {
    return logical(start);
}

// Logical operators
Node* logical(int start) {
    ind = start;
    Node* expr = equality(ind);

    while (expect_type(T_AND) != NULL || expect_type(T_OR) != NULL) {
        if (expr == NULL) {
            Error(tokens[ind - 1], "Unexpected token", 1);
        }
        int save = ind - 1;
        Node* right = equality(ind);
        if (right == NULL) {
            Error(tokens[ind - 1], "Expected expression after operator", 1);
        }
        expr = (Node*) new_Operator_node(tokens[save]->value, expr, right);
    }

    return expr;
}

// Equality operators
Node* equality(int start) {
    ind = start;
    Node* expr = comparison(ind);

    while (expect_type(T_NOT_EQUALS) != NULL || expect_type(T_EQUALS_EQUALS) != NULL) {
        if (expr == NULL) {
            Error(tokens[ind - 1], "Unexpected token", 1);
        }
        int save = ind - 1;
        Node* right = comparison(ind);
        if (right == NULL) {
            Error(tokens[ind - 1], "Expected expression after operator", 1);
        }
        expr = (Node*) new_Operator_node(tokens[save]->value, expr, right);
    }

    return expr;
}

// Comparative operators
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
            Error(tokens[ind - 1], "Expected right side of expression", 1);
        }
        expr = (Node*) new_Operator_node(tokens[save]->value, expr, right);
    }

    return expr;
}

// Addition or subtraction
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
            Error(tokens[ind - 1], "Expected right side of expression", 1);
        }
        expr = (Node*) new_Operator_node(tokens[save]->value, expr, right);
    }

    return expr;
}

// Multiplication or division
Node* factor(int start) {
    ind = start;
    Node* expr = unary(ind);

    while (expect_type(T_TIMES) != NULL || expect_type(T_DIVIDE) != NULL) {
        if (expr == NULL) {
            Error(tokens[ind - 1], "Unexpected token", 0);
        }
        int save = ind - 1;
        Node* right = unary(ind);
        if (right == NULL) {
            Error(tokens[ind - 1], "Expected right side of expression", 1);
        }
        expr = (Node*) new_Operator_node(tokens[save]->value, expr, right);
    }

    return expr;
}

// Negative numbers (-1) or opposite booleans (not true)
Node* unary(int start) {
	ind = start;

    if (expect_type(T_MINUS) != NULL || expect_type(T_NOT) != NULL) {
        int save = ind - 1;
        Node* right = unary(ind);
        if (right == NULL) {
            Error(tokens[save], "Expected something after operator", 1);
        }
        return (Node*) new_Operator_node(tokens[save]->type == T_MINUS ? "*" : "!=", (Node*) new_Integer_node(tokens[save]->type == T_MINUS ? -1 : 1), right);
    }

    return primary(ind);
}

// Goes through all the possible primary operands
Node* primary(int start) {
	ind = start;

    if (expect_type(T_TRUE) != NULL) {
        return (Node*) new_Boolean_node(1);
    }

    if (expect_type(T_FALSE) != NULL) {
        return (Node*) new_Boolean_node(0);
    }
    
	if (expect_type(T_INT) != NULL) {
        return (Node*) new_Integer_node(atoi(tokens[ind - 1]->value));
    }

    if (expect_type(T_ID) != NULL) {
        int begin = ind - 1;
        if (symtab_find_global(tokens[begin]->value, "var") == NULL) {
            Error(tokens[begin], "Use of undefined variable", 0);
        }
        return (Node*) new_Identifier_node(tokens[begin]->value, symtab_find_global(tokens[begin]->value, "var")->cgid, symtab_find_global(tokens[begin]->value, "var")->type);
    }

    if (expect_type(T_REAL) != NULL) {
        return (Node*) new_Real_node(strtof(tokens[ind - 1]->value, NULL));
    }

    if (expect_type(T_CHAR) != NULL) {
        return (Node*) new_Char_node(tokens[ind - 1]->value[0]);
    }

    if (expect_type(T_STR) != NULL) {
        return (Node*) new_String_node(tokens[ind - 1]->value);
    }

    if (expect_type(T_LEFT_PAREN) != NULL) {
        Node* expr = expression(ind);
        consume(T_RIGHT_PAREN, "Expect ')' after expression");
        return expr;
    }

	ind = start;
	return NULL;
}

// Function that parses variable declarations
Node* var_declaration(int start) {
    ind = start;
	// Expect a type (E.G. int, string, bool)
    char* type = expect_type(T_TYPE);
    if (type == NULL) {
        ind = start;
        return NULL;
    }
	// Expecting an identifier
    char* id = expect_type(T_ID);
    if (id == NULL) {
        Error(tokens[ind - 1], "Expected identifier after type\n", 1);
    }
	// Expecting an equal sign
    char* eq = expect_type(T_EQUALS);
    if (eq == NULL) {
        ind = start;
        return NULL;
    }
	// Expecting an expression
    Node* value = expression(ind);
    if (value == NULL) {
        Error(tokens[ind - 1], "Expected expression after equal sign\n", 1);
    }
    consume(T_SEMI_COLON, "Expected semi-colon");

	return (Node*) new_Var_declaration_node(id, str_format(".%d", id_c++), type, value);
}

// Tries all statements and returns the one that works
Node* statement(int start) {
    Node* base_node;
    base_node = var_declaration(start);
    if (base_node != NULL) {
        return base_node;
    }
    return NULL;
}

// Continuously calls statement()
void program(Node** ast, int max_len) {
    if (max_len == -1) {
        max_len = tokens_len;
    }
    int stmt_c = 0;
	// Iterate through tokens
    while (ind < max_len) {
        Node* stmt = statement(ind);
		// If no statements work, give error
        if (stmt == NULL) {
            Error(tokens[ind - 1], "Unexpected token\n", 0);
        }
        ast[stmt_c++] = stmt;
    }
}
// Main parse function
void parse(char* code, Token** toks, Node** ast, Symbol** sym_tab) {
    // Save a global length of tokens
    tokens_len = toks_len(toks);

    // Copy tokens
    tokens = malloc(tokens_len * sizeof(Token*));
    memcpy(tokens, toks, tokens_len * sizeof(Token*));

    program(ast, -1);
}
