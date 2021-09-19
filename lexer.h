#ifndef LEXER_H
#define LEXER_H

typedef enum {
    NUM_T,
    ID_T,
    STR_T,

    ADD_T,
    SUB_T,
    MUL_T,
    DIV_T,
} Token_t;

typedef struct {
    Token_t type;
    char* value;
    int lineno;
    int col;
} Token;

void lex(char* code, Token** tokens);

#endif // LEXER_H
