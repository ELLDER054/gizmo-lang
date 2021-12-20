#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"
#include "parser.h"
#include "../back-end/codegen.h"
#include "../common/include/log.h"

/**
 * @brief Checks if the given character is a valid start of an identifier
 *
 * @param c character to check
 *
 * @return non-zero if valid, zero otherwise
 */
int isAlpha(char c) { 
    return (c >= 'a' && c <= 'z') || c == '_' || (c >= 'A' && c <= 'Z');
}

/**
 * @brief Checks if the given character is a digit
 *
 * @param c character to check
 *
 * @return non-zero if digit, zero otherwise
 */
int isDigit(char c) {
    return (c >= '0' && c <= '9');
}

/**
 * @brief Checks if the given character is a token
 *
 * @param c character to check
 *
 * @return non-zero if valid token, zero otherwise
 */
int is_one_char_token(char c) {
    return (c == '(') || (c == ')') || (c == '[') || (c == ']') || (c == '{') || (c == '}') || (c == '.') || (c == ',') || (c == ':') || (c == ';');
}

/**
 * @brief Converts a character to a token
 *
 * @param c character to convert
 *
 * @return token converted from given character
 */
int one_char_tokens(char c) {

    /* Check if c is any of ()[]{}.,:; */
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

/**
 * @brief Returns next character in code unless next is past the length of code
 *
 * @param code input string of current line
 * @param pos integer index of current line
 *
 * @return next character in the input line
 */
char next(char* code, int pos) {
    if (pos + 1 >= strlen(code)) {
        return ' ';
    }
    return code[pos + 1];
}

/**
 * @brief Returns the correct operator token based on current and next chars
 *
 * This function handles multi-character operators, such as '++'. It does this
 * by looking at the current and next characters from the input.
 *
 * @param ch current character
 * @param nch next character
 *
 * @return the correct token based on the current and next character
 */
int operators(char ch, char nch) {
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

/**
 * @brief Checks if parameter @p c is an operator
 *
 * @param c character to check
 *
 * @return non-zero is given parameter is an operator, zero otherwise
 */
int in_operators(char c) { /* Checks if variable `c` is an operator */
    return (c == '+') || (c == '-') || (c == '*') || (c == '/') || (c == '>') || (c == '<') || (c == '%');
}

/**
 * @brief Split the given txt into an array of tokens based on the delimeter
 * 
 * @todo: Rewrite this function to use strtok
 * 
 * @param txt[in] input text to split
 * @param delim[in] delimeter to use
 * @param tokens[out] array of tokens split from the given input 
 *
 * @return number of tokens split from the input
 */
int split(const char *txt, char delim, char ***tokens) {
    int *tklen, *t, count = 1;
    char **arr, *p = (char*) txt;

    // count the total number of tokens to split
    while (*p != '\0') if (*p++ == delim) {
        count++;
    }

    // allocate an int array to store the length of each token
    t = tklen = calloc(count, sizeof (int));
    for (p = (char*) txt; *p != '\0'; p++) {
        *p == delim ? *t++ : (*t)++;
    }

    // allocate an array of token pointers
    *tokens = arr = malloc(count * sizeof(char*));
    t = tklen;

    // allocate space for the first token string
    p = *arr++ = calloc(*(t++) + 1, sizeof(char*));
    while (*txt != '\0') {
        if (*txt == delim) {
            // allocate space for the next token string
            p = *arr++ = calloc(*(t++) + 1, sizeof(char*));
            txt++;
        } else {
            *p++ = *txt++;
        }
    }

    // free the array of token lengths
    free(tklen);

    return count;
}


/**
 * @brief Convert escaped characters in a string to LLVM string format
 *
 * @todo: Fix this
 *
 * @param str string to convert
 * @param endstr[out] converted string
 * @param is_in_str whether this is a string literal or not 
 */
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

/**
 * @brief Convert the input code to an array of tokens (i.e., the lexer)
 *
 * @param code[in] string contain the code to lex
 * @param tokens[out] array of tokens lexed from the input
 */
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
            long long int len = 0;
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
            } else if (strcmp(name, "not") == 0) {
                tok.type = T_NOT;
            } else if (strcmp(name, "true") == 0) {
                tok.type = T_TRUE;
            } else if (strcmp(name, "while") == 0) {
                tok.type = T_WHILE;
            } else if (strcmp(name, "for") == 0) {
                tok.type = T_FOR;
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
            tok.line = strdup(lines[lineno - 1]);
            tok.col = begin;
            tokens[token_c++] = tok;
        } else if (isDigit(ch)) {
            long long int len = 0;
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
                num[numpos++] = '.';
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
                    fake_tok.line = strdup(lines[lineno - 1]);
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
            tok.line = strdup(lines[lineno - 1]);
            tok.col = begin;
            tokens[token_c++] = tok;
            free(num);
        } else if (is_one_char_token(ch)) {
            Token tok;
            tok.type = one_char_tokens(ch);
            tok.value = str_format("%c", ch);
            tok.lineno = lineno;
            tok.line = strdup(lines[lineno - 1]);
            tok.col = col++;
            pos++;
            tokens[token_c++] = tok;
        } else if (ch == '\\' && next(code, pos) == '(') {
            pos += 2;
            col += 2;
            ch = code[pos];
            while (1) {
                if (ch == '\0') {
                    Token fake_tok;
                    fake_tok.type = T_INT;
                    fake_tok.value = str_format(" ");
                    fake_tok.lineno = lineno;
                    fake_tok.line = strdup(lines[lineno - 1]);
                    fake_tok.col = col;
                    Error(fake_tok, "Expected end of comment", 0);
                } else if (ch == '\\' && next(code, pos) == ')') {
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
            tok.line = strdup(lines[lineno - 1]);
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
            tok.value = malloc(MAX_OPER_LEN);
            memset(tok.value, 0, MAX_OPER_LEN);
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
            tok.line = strdup(lines[lineno - 1]);
            tok.lineno = lineno;
            tokens[token_c++] = tok;
        } else if (ch == '!') {
            Token tok;
            if (next(code, pos) == '=') {
                tok.value = str_format("!=");
                tok.type = T_NOT_EQUALS;
                col++;
                pos++;
            } else {
                printf("In dev Error: Expected `=` after `!`");
                exit(-1);
            }
            tok.col = col++;
            pos++;
            tok.line = strdup(lines[lineno - 1]);
            tok.lineno = lineno;
            tokens[token_c++] = tok;
        } else if (ch == '"' || ch == '\'') {
            char delim = ch;
            long long int len = 0;
            int strpos = pos;
            ch = code[++strpos];
            while (ch != delim) {
                if (ch == '\n' || ch == '\0') {
                    Token fake_tok;
                    fake_tok.type = T_INT;
                    fake_tok.value = str_format(" ");
                    fake_tok.lineno = lineno;
                    fake_tok.line = strdup(lines[lineno - 1]);
                    fake_tok.col = col;
                    Error(fake_tok, "Expected end of string", 0);
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
            tok.value = malloc(len * 3 + 1);
            memset(tok.value, 0, len * 3 + 1);
            parse_string(string, tok.value, delim == '"'); 
            free(string);
            tok.type = tok_type;
            tok.lineno = lineno;
            tok.line = strdup(lines[lineno - 1]);
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
            Token fake_tok;
            fake_tok.type = T_INT;
            fake_tok.value = str_format(" ");
            fake_tok.lineno = lineno;
            fake_tok.line = strdup(lines[lineno - 1]);
            fake_tok.col = col;
            Error(fake_tok, str_format("Unexpected character '%c'", ch), 0);
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
