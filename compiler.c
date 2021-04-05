#include <stdio.h>
#include <string.h>
#include "scanner.h"

void scan(char code[1000], Token buf_toks[1000]);

int main(void) {
    char code[1000] = "'a";
    Token tokens[strlen(code)];
    scan(code, tokens);
    char a[3];
    return 0;
}
