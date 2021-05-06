#include <stdio.h>
#include <string.h>
#include "ast.h"

typedef struct Item;

char empty[1024]

typedef struct {
    Item* next;
    char* key;
    char* val;
} Item;

typedef struct {
    Item* begin;
} Dict;

Item* new_Item(Item* next, char* key, char* val) {
    Item* i = malloc(sizeof(Item));
    i->next = next;
    strcpy(i->key, key);
    strcpy(i->val, val);
    return i;
}

Dict* new_Dict() {
    Dict* d = malloc(sizeof(Dict));
    d->begin = new_Item(NULL, empty, empty);
    return d;
}

void free_item(Item* i) {
    free_item(i->next);
    free(i);
}

void generate(Node** ast, char* code) {
    Dict* vars = new_Dict();
    Item* cur_item = new_Item();
    for (int i = 0; i < sizeof(ast) / sizeof(Node*); i++) {
        Node* cur_node = ast[i];
        if (cur_node->n_type == VAR_DECLARATION_NODE) {
            char* t = generate_expr(((Var_declaration_node*) cur_node)->value);
            cur_item->next->key = t;
            cur_item->next->val = ((Var_declaration_node*) cur_node)->name;
            cur_item = cur_item->next;
        }
    }
    free_item(d->begin);
    free(d);
}
