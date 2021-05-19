#ifndef STRT_H
#define STRT_H

typedef struct {
    char** keys;
    char** vals;
} Dict;

Dict* dict_new();
void dict_add(Dict* dict, char* key, char* val);
char* dict_find(Dict* dict, char* key);
void dict_end_use(Dict* dict);

#endif // STRT_H
