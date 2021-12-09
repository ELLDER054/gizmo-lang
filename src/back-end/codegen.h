#ifndef codegen_h
#define codegen_h

#include "../front-end/ast.h"
#include "../streambuf.h"

void generate(Node** ast, int size, Stream_buf* code, char* file_name);
char* str_format(const char* fmt, ...);

#endif
