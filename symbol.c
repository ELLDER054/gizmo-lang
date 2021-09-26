#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol.h"

struct SymbolTable {
  SymbolTable* parent;
  SymbolTable* child;
  Symbol* sym_head;
  Symbol* sym_tail;
};

// Global scope
static SymbolTable *global = NULL;
// Current scope
static SymbolTable *cur = NULL;

// Initializes a symbol table
void symtab_init(void) {
  global = malloc(sizeof(SymbolTable));
  memset(global, 0, sizeof(SymbolTable));
  cur = global;
}

// Print the symbols in the current scope
void symtab_print_all(void) {
  Symbol *sym = cur->sym_head;
  SymbolTable *parent = cur->parent;
  while (sym != NULL) {
    Symbol *next = sym->next;
    printf("SYM: name (%s), type (%s), symtype (%s), cgid (%s)\n", sym->name, sym->type, sym->sym_type, sym->cgid);
    sym = next;
  }
}

// Allows other files to access the current scope
SymbolTable* symtab_get_current() {
    return cur;
}

// Get the info from a symbol
Symbol* symtab_find_in(SymbolTable* symtab, char* name, char* sym_type) {
  for (SymbolTable* sym_tab = symtab; sym_tab != NULL; sym_tab = sym_tab->parent) {
    for (Symbol *sym = sym_tab->sym_head; sym != NULL; sym = sym->next) {
      if (strcmp(sym->name, name) == 0 && strcmp(sym->sym_type, sym_type) == 0) {
        return sym;
      }
    }
  }
  return NULL;
} 

// Frees a symbol table
void symtab_destroy(void) {
  Symbol* sym = global->sym_head;
  while (sym != NULL) {
    Symbol *next = sym->next;
    free(sym->name);
    free(sym->type);
    free(sym->sym_type);
    free(sym->cgid);
    free(sym);
    sym = next;
  }
   free(global);
}

// Finds a symbol in the local scope
Symbol* symtab_find_local(char *name, char* sym_type) {
  for (Symbol *sym = cur->sym_head; sym != NULL; sym = sym->next) {
    if (strcmp(sym->name, name) == 0 && strcmp(sym->sym_type, sym_type) == 0) {
      return sym;
    }
  }
  return NULL;
}

// Finds a symbol in the global scope
Symbol* symtab_find_global(char *name, char* sym_type) {
  for (SymbolTable* sym_tab = cur; sym_tab != NULL; sym_tab = sym_tab->parent) {
    for (Symbol *sym = sym_tab->sym_head; sym != NULL; sym = sym->next) {
      if (strcmp(sym->name, name) == 0 && strcmp(sym->sym_type, sym_type) == 0) {
        return sym;
      }
    }
  }
  return NULL;
}

// Adds a symbol to the local scope
int symtab_add_symbol(char* type, char* sym_type, char* name, int args_len, char* cgid) {
  if (NULL != symtab_find_local(name, sym_type)) {
    printf("Symbol %s already exists!\n", name);
    return -1;
  }

  Symbol* sym = malloc(sizeof(Symbol));
  if (NULL == sym) {
    printf("Ran out of memory\n");
    return -1;
  }

  sym->name = strdup(name);
  sym->type = strdup(type);
  sym->sym_type = strdup(sym_type);
  sym->cgid = strdup(cgid);
  sym->args_len = args_len;
  sym->next = NULL;

  if (NULL == cur->sym_head) {
    cur->sym_head = sym;
  }

  if (NULL != cur->sym_tail) {
    cur->sym_tail->next = sym;
  }
  cur->sym_tail = sym;

  return 0;
}

// Pushes a new context to the symbol table
void symtab_push_context(void) {
  SymbolTable* sym_tab = malloc(sizeof(SymbolTable));
  memset(sym_tab, 0, sizeof(SymbolTable));
  cur->child = sym_tab;
  sym_tab->parent = cur;
  cur = sym_tab;
}

// Pops a context from the symbol table
void symtab_pop_context(void) {
  Symbol *sym = cur->sym_head;
  SymbolTable *parent = cur->parent;
  while (sym != NULL) {
    Symbol *next = sym->next;
    free(sym->name);
    free(sym->type);
    free(sym->sym_type);
    free(sym->cgid);
    free(sym);
    sym = next;
  }
  free(cur);
  cur = parent;
}
