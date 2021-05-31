#ifndef SYMBOLS_H
#define SYMBOLS_H

struct Symbol;
struct SymbolTable;

typedef struct SymbolTable SymbolTable;
typedef struct Symbol Symbol;

struct Symbol {
    char* name;
    char* type;
    char* sym_type;
    int args_len;
    Symbol* next;
};


Symbol* symtab_find_local(char *name, char* sym_type);
Symbol* symtab_find_global(char *name, char* sym_type);
int symtab_add_symbol(char* type, char* sym_type, char* name, int args_len);
void symtab_push_context(void);
void symtab_pop_context(void);
void symtab_init(void);
void symtab_destroy(void);

#endif /* SYMBOLS_H */
