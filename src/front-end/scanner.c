#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"
#include "parser.h"
#include "../common/include/log.h"

int isAlpha(char c) { /* Checks if the given character is a valid part of an identifier */
    return (c >= 'a' && c <= 'z') || c == '_' || (c >= 'A' && c <= 'Z');
}

/* For both isAlpha() and isDigit(), we can compare variable 'c' to digits due to the fact that chars are digits */

int isDigit(char c) { /* Checks if the given character is a valid digit */
    return (c >= '0' && c <= '9');
}

int is_one_char_token(char c) { /* Checks if the given character is a one-char-token */
    return (c == '(') || (c == ')') || (c == '[') || (c == ']') || (c == '{') || (c == '}') || (c == '.') || (c == ',') || (c == ':') || (c == ';');
}

int one_char_tokens(char c) { /* Returns tokens based on given character */
    switch(c) { /* Check if c is any of ()[]{}.,:; */
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

char next(char* code, int pos) { /* Returns next character in code unless the next is past the length of code */
    if (pos + 1 >= strlen(code)) {
        return ' ';
    }
    return code[pos + 1];
}

int operators(char ch, char nch) { /* Returns operator token based on the current and next characters */
    /* plus operators */
    if (ch == '+' && nch == '+') {
        return T_PLUS_PLUS;
    } else if (ch == '+' && nch == '=') {
        return T_PLUS_EQUALS;
    } else if (ch == '+') {
        return T_PLUS;
    }

    /* minus operators */
    if (ch == '-' && nch == '-') {
        return T_MINUS_MINUS;
    } else if (ch == '-' && nch == '=') {
        return T_MINUS_EQUALS;
    } else if (ch == '-') {
        return T_MINUS;
    }

    /* times operators */
    if (ch == '*' && nch == '*') {
        return T_TIMES_TIMES;
    } else if (ch == '*' && nch == '=') {
        return T_TIMES_EQUALS;
    } else if (ch == '*') {
        return T_TIMES;
    }

    /* divide operators */
    if (ch == '/' && nch == '/') {
        return T_DIVIDE_DIVIDE;
    } else if (ch == '/' && nch == '=') {
        return T_DIVIDE_EQUALS;
    } else if (ch == '/') {
        return T_DIVIDE;
    }

    /* modulo operators */
    if (ch == '%' && nch == '=') {
        return T_MOD_EQUALS;
    } else if (ch == '%') {
        return T_MOD;
    }

    /* boolean operators */
    if (ch == '>' && nch == '=') {
        return T_GREATER_THAN_EQUALS;
    } else if (ch == '>') {
        return T_GREATER_THAN;
    } else if (ch == '<' && nch == '=') {
        return T_LESS_THAN_EQUALS;
    } else if (ch == '<') {
        return T_LESS_THAN;
    } else {
        return 0;
    }
}

int in_operators(char c) { /* Checks if variable `c` is an operator */
    return (c == '+') || (c == '-') || (c == '*') || (c == '/') || (c == '>') || (c == '<') || (c == '%');
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

void parse_string(char* str, char* endstr, int is_in_str) {
    int pos = 0;
    strcpy(endstr, "");
    while (pos < strlen(str)) {
        char c = str[pos];
        if (c == '\\') {
            switch (str[pos + 1]) {
                case '\\':
                    strcat(endstr, "\\\\");
                    break;
                case 'n':
                    if (is_in_str) {
                        strcat(endstr, "\\0A");
                    } else {
                        strcat(endstr, "\n");
                    }
                    break;
                case 't':
                    if (is_in_str) {
                        strcat(endstr, "\\09");
                    } else {
                        strcat(endstr, "\t");
                    }
                    break;
                case '"':
                    if (is_in_str) {
                        strcat(endstr, "\\22");
                    } else {
                        strcat(endstr, "\"");
                    }
                    break;
                case '\'':
                    if (is_in_str) {
                        strcat(endstr, "\\27");
                    } else {
                        strcat(endstr, "'");
                    }
                    break;
                default:
                    strcat(endstr, "\\\\");
                    strncat(endstr, &(str[pos + 1]), 1);
            }
            pos++;
        } else {
            strncat(endstr, &c, 1);
        }
        pos++;
    }
    if (is_in_str) {
        strcat(endstr, "\\00");
    }
}

void scan(char* code, Token* tokens) {
    int lineno = 1;
    char** lines;
    int count;
    int i;
    count = split(code, '\n', &lines);
    int pos = 0;
    int token_c = 0;
    int col = 0;
    
    while (pos < strlen(code)) {
        Token tok;
        char ch = code[pos];
        if (isAlpha(ch)) {
            int len = 0;
            int idpos = pos;
            while (isAlpha(ch) || isDigit(ch)) {
                ch = code[++idpos];
                len++;
            }
            len++;

            char* name = malloc(len);
            memset(name, 0, len);

            int begin = col;
            ch = code[pos];
            idpos = 0;
            while (isAlpha(ch) || isDigit(ch)) {
                name[idpos++] = ch;
                ch = code[++pos];
                col++;
            }
            name[idpos] = '\0';

            Token tok;
            if (strcmp(name, "int") == 0 || strcmp(name, "string") == 0 || strcmp(name, "char") == 0 || strcmp(name, "real") == 0 || strcmp(name, "auto") == 0 || strcmp(name, "bool") == 0) { /* Is a type */
                tok.type = T_TYPE;
            } else if (strcmp(name, "return") == 0) /* Is return */ {
                tok.type = T_RETURN;
            } else if (strcmp(name, "and") == 0) {
                tok.type = T_AND;
            } else if (strcmp(name, "or") == 0) {
                tok.type = T_OR;
            } /*else if (strcmp(name, "not") == 0) {
                tok.type = T_NOT;
            }*/ else if (strcmp(name, "true") == 0) {
                tok.type = T_TRUE;
            } else if (strcmp(name, "while") == 0) {
                tok.type = T_WHILE;
            } else if (strcmp(name, "if") == 0) {
                tok.type = T_IF;
            } else if (strcmp(name, "else") == 0) {
                tok.type = T_ELSE;
            } else if (strcmp(name, "false") == 0) {
                tok.type = T_FALSE;
            } else if (strcmp(name, "break") == 0 || strcmp(name, "continue") == 0) {
                tok.type = T_SKIP;
            } else if (strcmp(name, "not") == 0) {
                tok.type = T_NOT;
            } else { /* Is an identifier */
                tok.type = T_ID;
            }
            tok.lineno = lineno;
            tok.value = malloc(len);
            memset(tok.value, 0, len);
            strcpy(tok.value, name);
            free(name);
            strcpy(tok.line, lines[lineno - 1]);
            tok.col = begin;
            tokens[token_c++] = tok;
        } else if (isDigit(ch)) {
            int len = 0;
            int numpos = pos;
            while (isDigit(ch) || ch == '.') {
                ch = code[++numpos];
                len++;
            }
            len++;

            char* num = malloc(len);
            memset(num, 0, len);

            TokenType tok_type = T_INT;
            int begin = col;
            ch = code[pos];
            numpos = 0;
            while (isDigit(ch)) {
                num[numpos++] = ch;
                ch = code[++pos];
                col++;
            }
            if (ch == '.') {
                ch = code[++pos];
                col++;
                strcat(num, ".");
                while (isDigit(ch)) {
                    num[numpos++] = ch;
                    ch = code[++pos];
                    col++;
                }
                if (ch == '.') {
                    Token fake_tok;
                    fake_tok.type = T_INT;
                    fake_tok.value = malloc(len);
                    memset(fake_tok.value, 0, len);
                    strcpy(fake_tok.value, " ");
                    fake_tok.lineno = lineno;
                    strcpy(fake_tok.line, lines[lineno - 1]);
                    fake_tok.col = col;
                    Error(fake_tok, "Too many dots in floating point number", 0);
                } else {
                    tok_type = T_REAL;
                }
            }

            Token tok;
            tok.value = malloc(len);
            memset(tok.value, 0, len);
            tok.type = tok_type;
            strcpy(tok.value, num);
            tok.lineno = lineno;
            strcpy(tok.line, lines[lineno - 1]);
            tok.col = begin;
            tokens[token_c++] = tok;
            free(num);
        } else if (is_one_char_token(ch)) {
            Token tok;
            tok.type = one_char_tokens(ch);
            tok.value = malloc(2);
            memset(tok.value, 0, 2);
            tok.value[0] = ch;
            tok.value[1] = '\0';
            tok.lineno = lineno;
            strcpy(tok.line, lines[lineno - 1]);
            tok.col = col++;
            pos++;
            tokens[token_c++] = tok;
        } else if (ch == '\\' && next(code, pos) == '*') {
            pos += 2;
            col += 2;
            ch = code[pos];
            while (1) {
                if (ch == '\0') {
                    Token fake_tok;
                    fake_tok.type = T_INT;
                    fake_tok.value = malloc(2);
                    memset(fake_tok.value, 0, 2);
                    strcpy(fake_tok.value, " ");
                    fake_tok.lineno = lineno;
                    strcpy(fake_tok.line, lines[lineno - 1]);
                    fake_tok.col = col;
                    Error(fake_tok, "Expected end of comment", 0);
                } else if (ch == '*' && next(code, pos) == '/') {
                    break;
                }
                ch = code[++pos];
                col++;
            }
            col += 2;
            pos++;
            ch = code[pos++];
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
            tok.value = malloc(MAX_OPER_LEN);
            memset(tok.value, 0, MAX_OPER_LEN);
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
            tokens[token_c++] = tok;
        } else if (ch == '=') {
            Token tok;
            tok.value = malloc(3);
            memset(tok.value, 0, 3);
            if (next(code, pos) == '=') {
                strcpy(tok.value, "==");
                tok.type = T_EQUALS_EQUALS;
                col++;
                pos++;
            } else {
                strcpy(tok.value, "=");
                tok.type = T_ASSIGN;
            }
            tok.col = col++;
            pos++;
            strcpy(tok.line, lines[lineno - 1]);
            tok.lineno = lineno;
            tokens[token_c++] = tok;
        } else if (ch == '!') {
            Token tok;
            if (next(code, pos) == '=') {
                tok.value = malloc(3);
                memset(tok.value, 0, 3);
                strcpy(tok.value, "!=");
                tok.type = T_NOT_EQUALS;
                col++;
                pos++;
            } else {
                printf("In dev Error: Expected `=` after `!`");
                exit(-1);
            }
            tok.col = col++;
            pos++;
            strcpy(tok.line, lines[lineno - 1]);
            tok.lineno = lineno;
            tokens[token_c++] = tok;
        } else if (ch == '"' || ch == '\'') {
            char delim = ch;
            int len = 0;
            int strpos = pos;
            ch = code[++strpos];
            while (ch != delim) {
                if (ch == '\n' || ch == '\0') {
                    Token fake_tok;
                    fake_tok.type = T_INT;
                    fake_tok.value = malloc(2);
                    memset(fake_tok.value, 0, 2);
                    strcpy(fake_tok.value, " ");
                    fake_tok.lineno = lineno;
                    strcpy(fake_tok.line, lines[lineno - 1]);
                    fake_tok.col = col;
                    Error(fake_tok, "Expected end of string", 0);
                }
                if (code[pos - 1] != '\\' && ch == '\\' && next(code, strpos) == delim) {
                    strpos += 2;
                    ch = code[strpos];
                    len += 2;
                    continue;
                }
                ch = code[++strpos];
                len++;
            }
            len++;

            char* string = malloc(len);
            memset(string, 0, len);
            
            TokenType tok_type = T_STR;
            int begin = col;
            ch = code[++pos];
            strpos = 0;
            while (ch != delim) {
                if (code[pos - 1] != '\\' && ch == '\\' && next(code, pos) == delim) {
                    pos += 2;
                    col += 2;
                    ch = code[pos];
                    strcat(string, "\\");
                    string[strpos++] = ch;
                    continue;
                }
                string[strpos++] = ch;
                ch = code[++pos];
                col++;
            }
            string[strpos] = '\0';
            pos++;
            col++;

            Token tok;
            if (delim == '\'') {
                tok_type = T_CHAR;
            }
            tok.value = malloc(len * 3);
            memset(tok.value, 0, len * 3);
            parse_string(string, tok.value, delim == '"');
            free(string);
            tok.type = tok_type;
            tok.lineno = lineno;
            strcpy(tok.line, lines[lineno - 1]);
            tok.col = begin;
            tokens[token_c++] = tok;
        } else if (ch == ' ' || ch == '\t') {
            pos++;
            col++;
        } else if (ch == '\n') {
            pos++;
            col = 0;
            lineno++;
        } else {
            char* error = malloc(100);
            memset(error, 0, 100);
            snprintf(error, 100, "Unexpected character `%c`", ch);
            Token fake_tok;
            fake_tok.type = T_INT;
            fake_tok.value = malloc(2);
            memset(fake_tok.value, 0, 2);
            strcpy(fake_tok.value, " ");
            fake_tok.lineno = lineno;
            strcpy(fake_tok.line, lines[lineno - 1]);
            fake_tok.col = col;
            Error(fake_tok, error, 0);
        }
    }

    /*for (int i = 0; i < strlen(code); i++) {
        if (tokens[i].type < 200 || tokens[i].type > 247) {
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
