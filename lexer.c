#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"
#include "lexer.h"

// Returns true if c is in the alphabet or is an underscore
int is_alpha(char c) {
    return (c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A) || (c == 0x5F);
}

int is_digit(char c) {
    return c >= 0x30 && c <= 0x39;
}

Token* new_token(Token_t type, char* value, int lineno, int col) {
    Token* tok = malloc(sizeof(Token));
    memset(tok, 0, sizeof(Token));

    tok->type = type;
    tok->lineno = lineno;
    tok->col = col;

    tok->value = malloc(strlen(value) + 1);
    strcpy(tok->value, value);

    return tok;
}

void lex(char* code, Token** tokens) {
    // position in the code
    int pos = 0;

    // Current line number
    int lineno = 1;

    // Length from beginning of the line to current position
    int col = 0;

    // Token count
    int tok_c = 0;
    int code_len = strlen(code);
    
    while (pos < code_len) {
        char ch = code[pos];
        
        // Symbol tokens
        Token_t t = -1;
        Token_t double_t = -1;
        Token_t equal_t = -1;
        switch (ch) {
            case '+':
                t = T_PLUS;
                double_t = T_PLUS_PLUS;
                equal_t = T_PLUS_EQUALS;
                break;
            case '-':
                t = T_MINUS;
                double_t = T_MINUS_MINUS;
                equal_t = T_MINUS_EQUALS;
                break;
            case '*':
                t = T_TIMES;
                double_t = T_TIMES_TIMES;
                equal_t = T_TIMES_EQUALS;
                break;
            case '/':
                t = T_DIVIDE;
                double_t = T_DIVIDE_DIVIDE;
                equal_t = T_DIVIDE_EQUALS;
                break;
            case '=':
                t = T_EQUALS;
                double_t = T_EQUALS_EQUALS;
                // Because double_t and equal_t would be the same, we don't need to assign equal_t
                break;
            case ';':
                t = T_SEMI_COLON;
                break;
            case '(':
                t = T_LEFT_PAREN;
                break;
            case ')':
                t = T_RIGHT_PAREN;
                break;
            case '[':
                t = T_LEFT_BRACKET;
                break;
            case ']':
                t = T_RIGHT_BRACKET;
                break;
            case '{':
                t = T_LEFT_BRACE;
                break;
            case '}':
                t = T_RIGHT_BRACE;
                break;
            case '!':
                t = T_NOT;
                break;
        }
        if (t != -1) {
            if (double_t != -1 && code[pos + 1] == ch) {
                tokens[tok_c++] = new_token(double_t, str_format("%c%c", ch, code[pos + 1]), lineno, col++);
                pos++;
            } else if (equal_t != -1 && code[pos + 1] == '=') {
                tokens[tok_c++] = new_token(equal_t, str_format("%c=", ch), lineno, col++);
                pos++;
            } else {
                tokens[tok_c++] = new_token(t, str_format("%c", ch), lineno, col++);
            }
            pos++;
            continue;
        }
        
        if (is_alpha(ch)) {
            // Make a growable string
            Stream_buf* id = new_Stream_buf(NULL, 1);
            
            int begin = col;
            while (is_alpha(ch) || is_digit(ch)) {
                // Grow the string
                Stream_buf_append_str(id, str_format("%c", ch));
                ch = code[++pos];
                col++;
            }
            
            Token_t tok_t = T_ID;
            char* id_buf = (char*) (id->buf);
            if (strcmp(id_buf, "int") == 0 || strcmp(id_buf, "string") == 0 || strcmp(id_buf, "bool") == 0 || strcmp(id_buf, "char") == 0) {
                tok_t = T_TYPE;
            }

            tokens[tok_c++] = new_token(tok_t, (char*) (id->buf), lineno, begin);
        } else if (is_digit(ch)) {
            // Make a growable string
            Stream_buf* num = new_Stream_buf(NULL, 1);
            
            int begin = col;
            while (is_digit(ch)) {
                // Grow the string
                Stream_buf_append_str(num, str_format("%c", ch));
                ch = code[++pos];
                col++;
            }

            tokens[tok_c++] = new_token(T_INT, (char*) (num->buf), lineno, begin);
        } else if (ch == '"') {
            // Make a growable string
            Stream_buf* str = new_Stream_buf(NULL, 1);
            
            // Skip over first quote
            ch = code[++pos];
            int begin = col++;
            while (ch != '"') {
                // Grow the string
                Stream_buf_append_str(str, str_format("%c", ch));
                ch = code[++pos];
                col++;
            }

            // Skip over end quote
            pos++;
            col++;

            tokens[tok_c++] = new_token(T_STR, (char*) (str->buf), lineno, begin);
        } else if (ch == ' ' || ch == '\t') {
            // Skip over whitespace
            pos++;
            col++;
        } else if (ch == '\n') {
            // Increment lineno
            lineno++;
            pos++;
            // Reset col
            col = 0;
        } else {
            fprintf(stderr, "On line %d:\nerror: Undefined character '%c'\n", lineno, ch);
            break;
        }
    }

    // Print the tokens
    for (int i = 0; i < tok_c; i++) {
        printf("%d, ", tokens[i]->type);
        printf("%s, ", tokens[i]->value);
        printf("%d, ", tokens[i]->lineno);
        printf("%d\n", tokens[i]->col);
    }
}
