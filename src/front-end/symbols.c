#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symbols.h"

/**
 * Represents a symbol table
 */
struct SymbolTable {
  SymbolTable* parent;
  SymbolTable* child;
  Symbol* sym_head;
  Symbol* sym_tail;
};

/**
 * Pointer to the global symbol table
 */
SymbolTable *global = NULL;

/**
 * Pointer to the symbol table for the current local scope
 */
SymbolTable *cur = NULL;

/**
 * @brief Initialize a symbol table
 */
void symtab_init(void) {
  global = malloc(sizeof(SymbolTable));
  memset(global, 0, sizeof(SymbolTable));
  cur = global;
}

/**
 * @brief Print a symbol table
 */
void symtab_print_all(void) {
  Symbol *sym = cur->sym_head;
  SymbolTable *parent = cur->parent;
  while (sym != NULL) {
    Symbol *next = sym->next;
    printf("SYM: name (%s), type (%s), symtype (%s), cgid (%s)\n", sym->name, sym->type, sym->sym_type, sym->cgid);
    sym = next;
  }
}

/**
 * @brief Get the current symbol table
 *
 * @return 
 */
SymbolTable* symtab_get_current() {
    return cur;
}

/**
 * @brief Find a symbol by name and type
 *
 * @param symtab symbol table to search
 * @param name string name of the symbol to search for
 * @param sym_type type of symbol to search for
 *
 * @return pointer to symbol found or NULL if not found
 */
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

/**
 * @brief Free a symbol table
 */
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

/**
 * @brief Find a symbol in the current local scope
 *
 * @param name string name of symbol to search for
 * @param sym_type type of symbol to search for
 *
 * @return pointer to the symbol found or NULL if not found
 */
Symbol* symtab_find_local(char *name, char* sym_type) {
  for (Symbol *sym = cur->sym_head; sym != NULL; sym = sym->next) {
    if (strcmp(sym->name, name) == 0 && strcmp(sym->sym_type, sym_type) == 0) {
      return sym;
    }
  }
  return NULL;
}

/**
 * @brief Find a symbol in the global scope
 *
 * @param name name of symbol to search for
 * @param sym_type type of symbol to search for
 *
 * @return pointer to the symbol found or NULL if not found
 */
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

/**
 * @brief Add a symbol to the symbol table
 *
 * @param type string repsresenting the type of the symbol (i.e., "int")
 * @param sym_type whether this symbol is a variable or function
 * @param name string name of the symbol
 * @param args_len number of arguments (only used for function type symbols)
 * @param cgid code generation identifier
 *
 * @return 0 on success, non-zero otherwise
 */
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

/**
 * @brief Starts a new local scope
 */
void symtab_push_context(void) {
  SymbolTable* sym_tab = malloc(sizeof(SymbolTable));
  memset(sym_tab, 0, sizeof(SymbolTable));
  cur->child = sym_tab;
  sym_tab->parent = cur;
  cur = sym_tab;
}


/**
 * @brief Deletes the current local scope
 */
void symtab_pop_context(void) {
  /*Symbol *sym = cur->sym_head;*/
  SymbolTable *parent = cur->parent;
  /*while (sym != NULL) {
    Symbol *next = sym->next;
    free(sym->name);
    free(sym->type);
    free(sym->sym_type);
    free(sym->cgid);
    free(sym);
    sym = next;
  }
  free(cur);*/
  cur = parent;
}
