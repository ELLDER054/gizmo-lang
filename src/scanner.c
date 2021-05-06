#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

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
    switch(c) {
        case '(':
            return T_LEFT_PAREN;
        case ')':
            return T_RIGHT_PAREN;
        case '[':
            return T_LEFT_BRACKET;
        case ']':
            return T_RIGHT_BRACKET;
        case '{':
            return T_LEFT_BRACE;
        case '}':
            return T_RIGHT_BRACE;
        case '.':
            return T_DOT;
        case ',':
            return T_COMMA;
        case ':':
            return T_COLON;
        case ';':
            return T_SEMI_COLON;
    }
    return 0;
}

char next(char* code, int pos) {
    if (pos + 1 >= sizeof(code)) {
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
    }

    // minus operators
    if (ch == '-' && nch == '-') {
        return T_MINUS_MINUS;
    } else if (ch == '-' && nch == '=') {
        return T_MINUS_EQUALS;
    } else if (ch == '-') {
        return T_MINUS;
    }

    // times operators
    if (ch == '*' && nch == '*') {
        return T_TIMES_TIMES;
    } else if (ch == '*' && nch == '=') {
        return T_TIMES_EQUALS;
    } else if (ch == '*') {
        return T_TIMES;
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

int split(const char *txt, char delim, char ***tokens) {
    int *tklen, *t, count = 1;
    char **arr, *p = (char*) txt;

    while (*p != '\0') if (*p++ == delim) {
        count++;
    }
    t = tklen = calloc(count, sizeof (int));
    for (p = (char*) txt; *p != '\0'; p++) {
        *p == delim ? *t++ : (*t)++;
    }
    *tokens = arr = malloc(count * sizeof(char*));
    t = tklen;
    p = *arr++ = calloc(*(t++) + 1, sizeof(char*));
    while (*txt != '\0') {
        if (*txt == delim) {
            p = *arr++ = calloc(*(t++) + 1, sizeof(char*));
            txt++;
        } else {
            *p++ = *txt++;
        }
    }
    free(tklen);
    return count;
}

void repeat_c(char c, int n, char* string) {
    for (int i = 0; i < n; i++) {
        strncat(string, &c, 1);
    }
}

void scan(char* code, Token* tokens) {
    int lineno = 1;
    char** lines;
    int count, i;
    count = split (code, '\n', &lines);
    int pos = 0;
    int token_count = 0;
    int col = 0;

    while (pos < strlen(code)) {
        char ch = code[pos];
        if (isAlpha(ch)) {
            char name[1024] = {'\0'};

            int begin = col;
            while (isAlpha(ch) || isDigit(ch)) {
                strncat(name, &ch, 1);
                ch = code[++pos];
                col++;
            }

            Token tok;
            if (!strcmp(name, "int") || !strcmp(name, "string")) {
                    tok.type = T_TYPE;
            } else {
                tok.type = T_ID;
            }
            tok.lineno = lineno;
            strcpy(tok.value, name);
            strcpy(tok.line, lines[lineno - 1]);
            tok.col = begin;
            tokens[token_count++] = tok;
        } else if (isDigit(ch)) {
            char num[1024] = {'\0'};
            TokenType tok_type = T_INT;

            int begin = col;
            while (isDigit(ch)) {
                strncat(num, &ch, 1);
                ch = code[++pos];
                col++;
            }
            if (ch == '.' && isDigit(next(code, pos))) {
                ch = code[++pos];
                strncat(num, ".", 1);
                while (isDigit(ch)) {
                    strncat(num, &ch, 1);
                    ch = code[++pos];
                    col++;
                }
                if (ch == '.') {
                    char specifier[1024] = {'\0'};
                    repeat_c(' ', col, specifier);
                    strncat(specifier, "^", 1);
                    printf("On line %d:\nToo many dots in floating point number\n%s\n%s\n", lineno, lines[lineno - 1], specifier);
                    exit(0);
                } else {
                    tok_type = T_REAL;
                }
            }

            Token tok;
            tok.type = tok_type;
            strcpy(tok.value, num);
            tok.lineno = lineno;
            strcpy(tok.line, lines[lineno - 1]);
            tok.col = begin;
            tokens[token_count++] = tok;
        } else if (is_one_char_token(ch)) {
            Token tok;
            tok.type = one_char_tokens(ch);
            tok.value[0] = ch;
            tok.lineno = lineno;
            strcpy(tok.line, lines[lineno - 1]);
            tok.col = col++;
            pos++;
            tokens[token_count++] = tok;
        } else if (ch == '\\' && next(code, pos) == '(') {
            pos += 2;
            ch = code[pos];
            while (1) {
                if (ch != '\\' && next(code, pos) != ')') {
                    break;
                } else if (ch == '\0') {
                    char specifier[1024] = {'\0'};
                    repeat_c(' ', col, specifier);
                    strncat(specifier, "^", 1);
                    printf("On line %d:\nExpected closing comment\n%s\n%s\n", lineno, lines[lineno - 1], specifier);
                    exit(0);
                }
                ch = code[++pos];
                col++;
            }
            pos += 2;
            col += 2;
        } else if (ch == '\\') {
            pos++;
            col++;
            ch = code[pos];
            while (ch != '\n' && ch != '\0') {
                ch = code[++pos];
                col++;
            }
            col++;
            pos++;
        } else if (in_operators(ch)) {
            Token tok;
            tok.type = operators(ch, next(code, pos));
            tok.lineno = lineno;
            strcpy(tok.line, lines[lineno - 1]);
            tok.col = col;
            if (next(code, pos) == '=' || next(code, pos) == ch) {
                tok.value[0] = ch;
                char nch = next(code, pos);
                strncat(tok.value, &nch, 1);
                pos += 2;
                col += 2;
            } else {
                tok.value[0] = ch;
                pos++;
                col++;
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
            tok.col = col++;
            pos++;
            strcpy(tok.line, lines[lineno - 1]);
            tok.lineno = lineno;
            tokens[token_count++] = tok;
        } else if (ch == '"' || ch == '\'') {
            char string[1024] = {ch, '\0'};
            char delim = ch;
            TokenType tok_type = T_STR;

            int begin = col++;
            ch = code[++pos];
            while (ch != delim) {
                if (ch == '\n' || ch == '\0') {
                    char specifier[1024] = {'\0'};
                    repeat_c(' ', col, specifier);
                    strncat(specifier, "^", 1);
                    printf("On line %d:\nUnterminated string literal\n%s\n%s\n", lineno, lines[lineno - 1], specifier);
                    exit(0);
                }
                strncat(string, &ch, 1);
                ch = code[++pos];
                col++;
            }
            pos++;
            col++;

            Token tok;
            if (strlen(string) == 1) {
                tok_type = T_CHAR;
            }
            tok.type = tok_type;
            strncat(string, &delim, 1);
            strcpy(tok.value, string);
            tok.lineno = lineno;
            strcpy(tok.line, lines[lineno - 1]);
            tok.col = begin;
            tokens[token_count++] = tok;
        } else if (ch == ' ' || ch == '\t') {
            pos++;
            col++;
        } else if (ch == '\n') {
            pos++;
            col = 0;
            lineno++;
        } else {
            char specifier[1024] = {'\0'};
            repeat_c(' ', col, specifier);
            strncat(specifier, "^", 1);
            printf("On line %d:\nUnexpected character `%c`\n%s\n%s\n", lineno, ch, lines[lineno - 1], specifier);
            exit(0);
        }
    }

    /*for (int i = 0; i < strlen(code); i++) {
        if (tokens[i].type < 200 || tokens[i].type > 238) {
            break;
        }
        printf("%d", tokens[i].type);
        printf(", %s", tokens[i].value);
        printf(", %s", tokens[i].line);
        printf(", %d", tokens[i].lineno);
        printf(", %d\n", tokens[i].col);
    }*/
    for (i = 0; i < count; i++) free (lines[i]);
    free(lines);
}
