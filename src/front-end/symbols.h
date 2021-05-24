#ifndef SYMBOLS_H
#define SYMBOLS_H

typedef struct {
    char* name;
    char* type;
    char* sym_type;
    int args_len;
} Symbol;

int contains_symbol(Symbol* s);
Symbol* new_symbol(char* s_type, char* name, char* type, int args_len);
void push_symbol(char* type, char** info, int args_len);
Symbol* sym_find(char* name);

#endif /* SYMBOLS_H */
