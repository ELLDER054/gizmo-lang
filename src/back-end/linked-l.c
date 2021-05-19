#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char** list_new() {
    char** list = malloc(sizeof(char**));
    memset(list, 0, sizeof(char**));
    return list;
}

void list_add(char** list, char* val) {
    for (int i = 0; i < sizeof(list) / sizeof(char*); i++) {
        if (list[i] == NULL) {
            list[i] = val;
        }
    }
}

int list_len(char** list) {
    int len;
    for (len = 0; len < sizeof(list) / sizeof(char*); len++) {
        if (list[len] == NULL) {
            break;
        }
    }
    return len;
}

void list_end_use(char** list) {
    free(list);
}
