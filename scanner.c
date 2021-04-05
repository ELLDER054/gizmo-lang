#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

char* string_repeat(int n, const char* s) {
    size_t slen = strlen(s);
    char* dest = malloc(n * slen + 1);
    
    int i; char* p;
    for (i = 0, p = dest; i < n; ++i, p += slen) {
        memcpy(p, s, slen);
    }
    *p = '\0';
    return dest;
}

int isAlpha(char c) {
    return (c >= 65 && c <= 90) || c == '_' || (c >= 97 && c <= 122);
}

int isDigit(char c) {
    return (c >= 48 && c <= 57);
}

int is_one_char_token(char c) {
    return (c == '(') || (c == ')') || (c == '[') || (c == ']') || (c == '{') || (c == '}') || (c == '.') || (c == ',') || (c == ':') || (c == ';');
}

int one_char_tokens(char c) {
    if (c == '(') {
        return T_LEFT_PAREN;
    } else if (c == ')') {
        return T_RIGHT_PAREN;
    } else if (c == '[') {
        return T_LEFT_BRACKET;
    } else if (c == ']') {
        return T_RIGHT_BRACKET;
    } else if (c == '{') {
        return T_LEFT_BRACE;
    } else if (c == '}') {
        return T_RIGHT_BRACE;
    } else if (c == '.') {
        return T_DOT;
    } else if (c == ',') {
        return T_COMMA;
    } else if (c == ':') {
        return T_COLON;
    } else if (c == ';') {
        return T_SEMI_COLON;
    }
    return 0;
}

char next(char code[1000], int pos) {
    if (pos + 1 >= 1000) {
        return '_';
    }
    return code[pos + 1];
}

int operators(char ch, char nch) {
    // plus operators
    if (ch == '+' && nch == '+') {
        return T_PLUS_PLUS;
    } else if (ch == '+' && nch == '=') {
        return T_PLUS_EQUALS;
    } else if (ch == '+') {
        return T_PLUS;
    } else {
        return 0;
    }

    // minus operators
    if (ch == '-' && nch == '-') {
        return T_MINUS_MINUS;
    } else if (ch == '-' && nch == '=') {
        return T_MINUS_EQUALS;
    } else if (ch == '-') {
        return T_MINUS;
    } else {
        return 0;
    }

    // times operators
    if (ch == '*' && nch == '*') {
        return T_TIMES_TIMES;
    } else if (ch == '*' && nch == '=') {
        return T_TIMES_EQUALS;
    } else if (ch == '*') {
        return T_TIMES;
    } else {
        return 0;
    }

    // plus operators
    if (ch == '/' && nch == '/') {
        return T_DIVIDE_DIVIDE;
    } else if (ch == '/' && nch == '=') {
        return T_DIVIDE_EQUALS;
    } else if (ch == '/') {
        return T_DIVIDE;
    } else {
        return 0;
    }
}

int in_operators(char c) {
    return (c == '+') || (c == '-') || (c == '*') || (c == '/');
}

void scan(char code[1000], Token buf_toks[1000]) {
    int lineno = 1;
    int pos = 0;
    int token_count = 0;
    Token tokens[strlen(code)];

    while (pos < strlen(code)) {
        char ch = code[pos];
        if (isAlpha(ch)) {
            char name[200] = {'\0'};

            int begin = pos;
            while (isAlpha(ch) || isDigit(ch)) {
                strncat(name, &ch, 1);
                ch = code[++pos];
            }

            Token tok;
            if (!strcmp(name, "int") || !strcmp(name, "string")) {
                    tok.type = T_TYPE;
            } else {
                tok.type = T_ID;
            }
            tok.lineno = lineno;
            strcpy(tok.value, name);
            strcpy(tok.line, code);
            tok.col = begin;
            tokens[token_count++] = tok;
        } else if (isDigit(ch)) {
            char num[200] = {'\0'};
            int found_dot = 0;
            TokenType tok_type = T_INT;

            int begin = pos;
            while (isDigit(ch) || ch == '.') {
                if (ch == '.' && found_dot) {
                    printf("error: Too many dots in floating point number\n");
                    return;
                } else if (ch == '.' && !found_dot) {
                    found_dot = 1;
                    tok_type = T_REAL;
                }
                strncat(num, &ch, 1);
                ch = code[++pos];
            }

            Token tok;
            tok.type = tok_type;
            strcpy(tok.value, num);
            tok.lineno = lineno;
            strcpy(tok.line, code);
            tok.col = begin;
            tokens[token_count++] = tok;
        } else if (is_one_char_token(ch)) {
            Token tok;
            tok.type = one_char_tokens(ch);
            strcpy(tok.value, &ch);
            tok.lineno = lineno;
            strcpy(tok.line, code);
            tok.col = pos++;
            tokens[token_count++] = tok;
        } else if (in_operators(ch)) {
            Token tok;
            tok.type = operators(ch, next(code, pos));
            tok.lineno = lineno;
            strcpy(tok.line, code);
            if (next(code, pos) == '=' || in_operators(next(code, pos))) {
                tok.col = pos;
                strcpy(tok.value, &ch);
                char nch = next(code, pos);
                strncat(tok.value, &nch, 1);
                pos += 2;
            } else {
                tok.col = pos;
                pos++;
            }
            tokens[token_count++] = tok;
        } else if (ch == '=') {
            Token tok;
            if (next(code, pos) == '=') {
                strcpy(tok.value, "==");
                tok.type = T_EQUALS;
            } else {
                strcpy(tok.value, "=");
                tok.type = T_ASSIGN;
            }
            tok.col = pos++;
            strcpy(tok.line, code);
            tok.lineno = lineno;
            tokens[token_count++] = tok;
        } else if (ch == '"' || ch == '\'') {
            char string[1000] = {'\0'};
            char delim = ch;
            TokenType tok_type = T_STR;

            int begin = pos;
            ch = code[++pos];
            while (ch != delim) {
                if (ch == '\n' || ch == '\0') {
                    printf("error: Unexpected end of input\n");
                    return;
                }
                strncat(string, &ch, 1);
                ch = code[++pos];
            }
            pos++;

            Token tok;
            if (strlen(string) == 1) {
                tok_type = T_CHAR;
            }
            tok.type = tok_type;
            strcpy(tok.value, string);
            tok.lineno = lineno;
            strcpy(tok.line, code);
            tok.col = begin;
            tokens[token_count++] = tok;
        } else if (ch == ' ' || ch == '\t') {
            pos++;
        } else {
            printf("error: Unexpected character `%c`\n", ch);
            return;
        }
    }
    int buf_tok_c = 0;
    for (int i = 0; i < strlen(code); i++) {
        if (tokens[i].type < 200 || tokens[i].type > 237) {
            break;
        }
        printf("%d", tokens[i].type);
        printf(", %s", tokens[i].value);
        printf(", %s", tokens[i].line);
        printf(", %d", tokens[i].lineno);
        printf(", %d\n", tokens[i].col);
        buf_toks[buf_tok_c++] = tokens[i];
    }
}
