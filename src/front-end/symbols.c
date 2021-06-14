#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symbols.h"

struct SymbolTable {
  SymbolTable* parent;
  SymbolTable* child;
  Symbol* sym_head;
  Symbol* sym_tail;
};

SymbolTable *global = NULL;
SymbolTable *cur = NULL;

void symtab_init(void) { /* Initializes a symbol table */
  global = malloc(sizeof(SymbolTable));
  memset(global, 0, sizeof(SymbolTable));
  cur = global;
}

void symtab_print_all(void) {
  Symbol *sym = cur->sym_head;
  SymbolTable *parent = cur->parent;
  while (sym != NULL) {
    Symbol *next = sym->next;
    printf("SYM: name (%s), type (%s), symtype (%s), cgid (%s)\n", sym->name, sym->type, sym->sym_type, sym->cgid);
    sym = next;
  }
}

void symtab_destroy(void) { /* Frees a symbol table */
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

Symbol* symtab_find_local(char *name, char* sym_type) { /* Finds a symbol in the local scope */
  for (Symbol *sym = cur->sym_head; sym != NULL; sym = sym->next) {
    if (strcmp(sym->name, name) == 0 && strcmp(sym->sym_type, sym_type) == 0) {
      return sym;
    }
  }
  return NULL;
}

Symbol* symtab_find_global(char *name, char* sym_type) { /* Finds a symbol in the global scope */
  for (SymbolTable* sym_tab = cur; sym_tab != NULL; sym_tab = sym_tab->parent) {
    for (Symbol *sym = sym_tab->sym_head; sym != NULL; sym = sym->next) {
      if (strcmp(sym->name, name) == 0 && strcmp(sym->sym_type, sym_type) == 0) {
        return sym;
      }
    }
  }
  return NULL;
}

int symtab_add_symbol(char* type, char* sym_type, char* name, int args_len, char* cgid) { /* Adds a symbol to the local scope */
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

void symtab_push_context(void) { /* Pushes a new context to the symbol table */
  SymbolTable* sym_tab = malloc(sizeof(SymbolTable));
  memset(sym_tab, 0, sizeof(SymbolTable));
  cur->child = sym_tab;
  sym_tab->parent = cur;
  cur = sym_tab;
}

void symtab_pop_context(void) { /* Pops a context from the symbol table */
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
