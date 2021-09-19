#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tools.h"
#include "lexer.h"

int is_alpha(char c) { // Returns true if c is in the alphabet or is an underscore
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
    int pos = 0; // Position in the code
    int lineno = 1; // Current line number
    int col = 0; // Length from beginning of the line to current position
    int tok_c = 0; // Token count
    int code_len = strlen(code);
    
    while (pos < code_len) {
        char ch = code[pos];
        
        Token_t t = -1;
        switch (ch) {
            case '+':
                t = ADD_T;
                break;
            case '-':
                t = SUB_T;
                break;
            case '*':
                t = MUL_T;
                break;
            case '/':
                t = DIV_T;
                break;
        }
        if (t != -1) {
            pos++;
            tokens[tok_c++] = new_token(t, str_format("%c", ch), lineno, col++);
            continue;
        }

        if (is_alpha(ch)) { // If ch is in the alphabet or is an underscore
            Stream_buf* id = new_Stream_buf(NULL, 1); // Make a growable string
            
            int begin = col;
            while (is_alpha(ch) || is_digit(ch)) {
                Stream_buf_append_str(id, str_format("%c", ch)); // Grow the string
                ch = code[++pos];
                col++;
            }

            tokens[tok_c++] = new_token(ID_T, (char*) (id->buf), lineno, begin);
        } else if (is_digit(ch)) { // If ch is a digit
            Stream_buf* num = new_Stream_buf(NULL, 1); // Make a growable string
            
            int begin = col;
            while (is_digit(ch)) {
                Stream_buf_append_str(num, str_format("%c", ch)); // Grow the string
                ch = code[++pos];
                col++;
            }

            tokens[tok_c++] = new_token(NUM_T, (char*) (num->buf), lineno, begin);
        } else if (ch == '"') { // If ch is a double quote
            Stream_buf* str = new_Stream_buf(NULL, 1); // Make a growable string
            
            ch = code[++pos]; // Skip over first quote
            int begin = col++;
            while (ch != '"') {
                Stream_buf_append_str(str, str_format("%c", ch)); // Grow the string
                ch = code[++pos];
                col++;
            }
            pos++; // Skip over end quote
            col++;

            tokens[tok_c++] = new_token(STR_T, (char*) (str->buf), lineno, begin);
        } else if (ch == ' ' || ch == '\t') { // If ch is whitespace
            pos++; // Skip over whitespace
            col++;
        } else if (ch == '\n') { // If ch is a newline
            lineno++; // Increment lineno
            pos++;
            col = 0; // Reset col
        } else { // Unknown character
            fprintf(stderr, "On line %d:\nerror: Undefined character '%c'\n", lineno, ch);
            break;
        }
    }

    for (int i = 0; i < tok_c; i++) { // Print the tokens
        printf("%d, ", tokens[i]->type);
        printf("%s, ", tokens[i]->value);
        printf("%d, ", tokens[i]->lineno);
        printf("%d\n", tokens[i]->col);
    }
}
