#ifndef SCANNER_H
#define SCANNER_H

#define MAX_LINE_LEN 1024
#define MAX_NAME_LEN 100

typedef enum {
    T_INT = 200,
    T_REAL,
    T_STR,
    T_CHAR,

    // Identifiers
    T_ID,
    T_RETURN,
    T_TYPE,

    // Symbols
    T_LEFT_PAREN,
    T_RIGHT_PAREN,
    T_LEFT_BRACKET,
    T_RIGHT_BRACKET,
    T_LEFT_BRACE,
    T_RIGHT_BRACE,
    T_DOT,
    T_COMMA,
    T_COLON,
    T_SEMI_COLON,
    T_ASSIGN,
    T_EQUALS,
    T_EQUALS_EQUALS,
    T_NOT,
    T_AND,
    T_OR,
    T_NOT_EQUALS,
    T_LESS_THAN,
    T_GREATER_THAN,
    T_LESS_THAN_EQUALS,
    T_GREATER_THAN_EQUALS,
    T_PLUS,
    T_PLUS_EQUALS,
    T_PLUS_PLUS,
    T_MINUS,
    T_MINUS_EQUALS,
    T_MINUS_MINUS,
    T_TIMES,
    T_TIMES_EQUALS,
    T_TIMES_TIMES,
    T_DIVIDE,
    T_DIVIDE_EQUALS,
    T_DIVIDE_DIVIDE,
    T_MOD,
    T_MOD_EQUALS,
    T_TRUE,
    T_FALSE,
    T_WHILE,
    T_IF,
    T_ELSE,
    T_SKIP,
} TokenType;

typedef struct {
    int type;
    char* value;
    int col;
    char line[MAX_LINE_LEN];
    int lineno;
} Token;

void scan(char* code, Token* tokens);

#endif /* SCANNER_H */
