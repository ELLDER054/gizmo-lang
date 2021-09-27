#ifndef COMPILER_H
#define COMPILER_H

#include <stdint.h>

#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "symbol.h"

typedef struct {
    uint8_t* buf;
    int index;
    int len;
} Stream_buf;

Stream_buf* new_Stream_buf(uint8_t* buf, int len);
void Stream_buf_append_str(Stream_buf* s, char* to_append);
void Stream_buf_append(Stream_buf* s, uint8_t* to_append, int len);
void free_Stream_buf(Stream_buf* s);

char* str_format(const char* fmt, ...);

void Error(const Token* token, const char* err, const int after);

#endif // COMPILER_H
