#ifndef codegen_h
#define codegen_h

typedef struct Item;

typedef struct {
    Item* next;
    char* key;
    char* val;
} Item;

typedef struct {
    Item* begin;
} Dict;

#endif
