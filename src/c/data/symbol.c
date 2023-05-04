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

static symbol_t* new_symbol(char* name, bool interned) {
  symbol_t* out = allocate(sizeof(symbol_t));

  INIT_HEADER(out, SYMBOL, interned|FROZEN);
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

// object methods
void mark_symbol(void* ox) {
  symbol_t* symx = ox;

  obj_mark(symx->left);
  obj_mark(symx->right);
}

void free_symbol(void* ox) {
  symbol_t* symx = ox;

  deallocate(symx->name, strlen(symx->name) + 1);
}

usize sizeof_symbol(void* ox) {
  (void)ox;

  return sizeof(symbol_t);
}

uhash hash_symbol(void* ox, int bound, bool* oob) {
  (void)bound;
  (void)oob;

  symbol_t* symx = ox;

  if (!has_flag(symx, HASHED)) {
    uhash base     = TYPEHASH(ox);
    uhash namehash = hash_str(symx->name);
    uhash idnohash = hash_uword(symx->idno);
    uhash symhash  = mix_3_hashes(base, namehash, idnohash) & WVMASK;

    set_hash(symx, symhash);
  }

  return head(ox)->hash;
}

void print_symbol(value_t x, table_t* backrefs) {
  (void)backrefs;

  symbol_t* symx = as_symbol(x);

  if (has_flag(symx, INTERNED))
    printf("%s", symx->name);

  else
    printf("%s#%lu", symx->name, symx->idno);
}
