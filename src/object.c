#include <string.h>

#include "object.h"
#include "memory.h"

// APIs
// symbol
// globals
symbol_t* SymbolTable = NULL;
uint64 SymbolCounter = 1;

static symbol_t* new_symbol(char* name, bool interned) {
  symbol_t* out = allocate(sizeof(symbol_t));

  INIT_HEADER(out, &SymbolType, interned);
  out->name  = duplicate(name, strlen(name)+1, 0);
  out->left  = NULL;
  out->right = NULL;
  out->idno  = SymbolCounter++;

  return out;
}

static symbol_t** locate_symbol(char* name) {
  symbol_t** st = &SymbolTable;

  while (*st) {
    int o = strcmp(name, (*st)->name);

    if (o < 0)
      st = &(*st)->left;

    else if (o > 0)
      st = &(*st)->right;

    else
      break;
  }

  return st;
}

static symbol_t* intern_symbol(char* name) {
  symbol_t** location = locate_symbol(name);

  if (*location == NULL)
    *location = new_symbol(name, true);

  return *location;
}

symbol_t* symbol(char* name, bool intern) {
  symbol_t* out;
  
  if (intern) {
    assert(name);
    out = intern_symbol(name);
  } else {
    if (name == NULL)
      name = "symbol";

    out = new_symbol(name, false);
  }

  return out;
}

// list

