#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* string_new(char* val) {
    char* string = malloc(sizeof(val));
    memset(string, 0, sizeof(val));
    strcpy(string, val);
    return string;
}

void string_end_use(char* s) {
    free(s);
}
