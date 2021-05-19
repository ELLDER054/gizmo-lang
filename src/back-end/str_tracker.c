#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linked-l.h"
#include "mydict.h"

Dict* dict_new() {
    Dict* d = malloc(sizeof(Dict));
    d->keys = list_new();
    d->vals = list_new();
    return d;
}

void dict_add(Dict* dict, char* key, char* val) {
    list_add(dict->keys, key);
    list_add(dict->vals, val);
}

char* dict_find(Dict* dict, char* key) {
    for (int i = 0; i < list_len(dict->keys); i++) {
        if (strcmp(dict->keys[i], key) == 0) {
            return dict->vals[i];
        }
    }
    return "";
}

void dict_end_use(Dict* dict) {
    free(dict->keys);
    free(dict->vals);
    free(dict);
}
