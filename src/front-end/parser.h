#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "symbols.h"

void Error(const Token token, const char* error, const int after);
void parse(char* code, Token* tokens, Node** program, Symbol** symbol_table);

#endif /* PARSER_H */
