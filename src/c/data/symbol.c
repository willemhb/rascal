#include "data/symbol.h"

#include "runtime/memory.h"

#include "lang/hash.h"
#include "lang/print.h"

#include "util/io.h"
#include "util/string.h"
#include "util/hashing.h"

// globals
symbol_t* SymbolTable = NULL;
uint64 SymbolCounter = 1;

static symbol_t* new_symbol(char* name, flags fl) {
  symbol_t* out = allocate(sizeof(symbol_t));

  INIT_HEADER(out, SYMBOL, fl|FROZEN|(*name == ':')*LITERAL);
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

symbol_t* intern_symbol(char* name) {
  symbol_t** location = locate_symbol(name);

  if (*location == NULL)
    *location = new_symbol(name, true);

  return *location;
}

symbol_t* make_symbol(char* name, flags fl) {
  symbol_t* out;
  
  if (flagp(fl, INTERNED)) {
    assert(name);
    out = intern_symbol(name);
  } else {

    if (name == NULL)
      name = "symbol";

    out = new_symbol(name, 0);
  }

  return out;
}

// object methods
void trace_symbol(void* ox) {
  symbol_t* symx = ox;

  mark_object(symx->left);
  mark_object(symx->right);
}

void free_symbol(void* ox) {
  symbol_t* symx = ox;

  deallocate(symx->name, strlen(symx->name) + 1);
}

usize sizeof_symbol(void* ox) {
  (void)ox;

  return sizeof(symbol_t);
}

void print_symbol(value_t x, table_t* backrefs) {
  (void)backrefs;

  symbol_t* symx = as_symbol(x);

  if (object_hasfl(symx, INTERNED))
    printf("%s", symx->name);

  else
    printf("%s#%lu", symx->name, symx->idno);
}
