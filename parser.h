#ifndef PARSER_H
#define PARSER_H

#include "symbol.h"
#include "ast.h"
#include "lexer.h"

void parse(char* code, Token** tokens, Node** program, Symbol** symbol_table);

#endif /* PARSER_H */
