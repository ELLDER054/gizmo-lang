#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include "tools.h"

Stream_buf* new_Stream_buf(uint8_t* buf, int len) {
    Stream_buf* stream_buf = malloc(sizeof(Stream_buf));
    memset(stream_buf, 0, sizeof(Stream_buf));

    // Initialize stream buffer
    stream_buf->buf = malloc(len);
    memset(stream_buf->buf, 0, len);

    stream_buf->len = len;
    stream_buf->index = 0;

    return stream_buf;
}

void Stream_buf_append_str(Stream_buf* s, char* to_append) {
    // Rewind the NUL terminator
    if (s->index > 0) {
        s->index--;
    }

    Stream_buf_append(s, (uint8_t*) to_append, strlen(to_append) + 1);
}

void Stream_buf_append(Stream_buf* s, uint8_t* to_append, int len) {

    // If we need to reallocate
    if (len > (s->len - s->index)) {
        s->buf = realloc(s->buf, (s->len + len) * 2);
        s->len = (s->len + len) * 2;
    }

    // Add to_append to the stream buffer
    memcpy(s->buf + s->index, to_append, len);
    s->index += len;
}

void free_Stream_buf(Stream_buf* s) {
    free(s->buf);
    free(s);
}

// Returns a formatted string
char* str_format(const char* fmt, ...) {
    int size = 0;
    char *p = NULL;
    va_list ap;

    va_start(ap, fmt);
    size = vsnprintf(p, size, fmt, ap);
    va_end(ap);

    if (size < 0)
        return NULL;
    
    // For '\0'
    size++;
    p = malloc(size);
    if (p == NULL)
        return NULL;

    va_start(ap, fmt);
    size = vsnprintf(p, size, fmt, ap);
    va_end(ap);

    if (size < 0) {
        return NULL;
    }

    return p;
}

// Gives errors
void Error(const Token* token, const char* err, const int after) {
	fprintf(stderr, "\x1b[31;1merror\x1b[0m: On line %d\n%s\n", token->lineno, err);
    exit(-1);
}
