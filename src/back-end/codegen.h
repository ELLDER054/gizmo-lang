#ifndef codegen_h
#define codegen_h

#include "../front-end/ast.h"

void generate(Node** ast, int size, char* code, char* file_name);

#endif
