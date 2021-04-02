#include <stdio.h>
#include "scanner.h"

void scan(char code[1000]);

int main(void) {
    char code[1000] = "int a;";
    scan(code);
    return 0;
}
