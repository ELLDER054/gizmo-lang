#ifndef LEXER_H
#define LEXER_H

typedef enum {
    NUM_T,
    ID_T,
    TYPE_T,
    STR_T,

    ADD_T,
    SUB_T,
    MUL_T,
    DIV_T,
    EQUAL_T,

    ADD_ADD_T,
    SUB_SUB_T,
    MUL_MUL_T,
    DIV_DIV_T,
    EQUAL_EQUAL_T,

    ADD_EQUAL_T,
    SUB_EQUAL_T,
    MUL_EQUAL_T,
    DIV_EQUAL_T,

    SEMI_COLON_T,
    LEFT_PAREN_T,
    RIGHT_PAREN_T,
    LEFT_BRACKET_T,
    RIGHT_BRACKET_T,
    LEFT_BRACE_T,
    RIGHT_BRACE_T,

    NOT_T,
} Token_t;

typedef struct {
    Token_t type;
    char* value;
    int lineno;
    int col;
} Token;

void lex(char* code, Token** tokens);

#endif // LEXER_H
