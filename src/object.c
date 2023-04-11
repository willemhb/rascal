#include <string.h>
#include <tgmath.h>

#include "object.h"
#include "memory.h"
#include "number.h"

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

// table
#define MINC   8u
#define MAXC   0x0000800000000000ul
#define CLOADF 0.6125
#define NLOADF 1.6

table_t* table(void) {
  table_t* out = allocate(sizeof(table_t));

  init_table(out);
  return out;
}

static usize get_ords_size(usize cap) {
  usize out;
  
  if (cap <= INT8_MAX)
    out = cap * sizeof(sint8);

  else if (cap <= INT16_MAX)
    out = cap * sizeof(sint16);

  else if (cap <= INT32_MAX)
    out = cap * sizeof(sint32);

  else
    out = cap * sizeof(sint64);

  return out;
}

static usize get_entries_size(usize cap) {
  return cap * sizeof(value_t) << 1;
}

static void init_ords(void* o, usize total) {
  memset(o, -1, total);
}

static void init_entries(value_t* entries, usize start, usize stop) {
  for (usize i=start << 1; i <= (stop << 1); i++)
    entries[i] = NOTFOUND;
}

static void* allocate_ords(usize cap) {
  usize total = get_ords_size(cap);
  void* out   = allocate(total);
  init_ords(out, total);
  return out;
}

static value_t* allocate_entries(usize cap) {
  usize total  = get_entries_size(cap);
  value_t* out = allocate(total);
  init_entries(out, 0, cap);
  return out;
}

static void  deallocate_ords(void* o, usize cap) {
  usize total = get_ords_size(cap);
  deallocate(o, total);
}

static void deallocate_entries(value_t* entries, usize cap) {
  deallocate(entries, cap * sizeof(value_t) * 2);
}

static void* reallocate_ords(void* o, usize oldc, usize newc) {
  deallocate_ords(o, oldc);
  return allocate_ords(newc);
}

static value_t* reallocate_entries(value_t* entries, usize oldc, usize newc) {
  usize oldt = get_entries_size(oldc);
  usize newt = get_entries_size(newc);
  entries    = reallocate(entries, oldt, newt);

  if (newt > oldt)
    init_entries(entries, oldc, newc);
}

void        reset_table(table_t* slf) {
  deallocate_ords(slf->, slf->cap);
  deallocate_entries(slf->table);
  init_table(slf);
}

long        table_find(table_t* slf, value_t key);
value_t     table_get(table_t* slf, value_t key);
value_t     table_set(table_t* slf, value_t key, value_t val);
bool        table_add(table_t* slf, value_t key, value_t val);
bool        table_del(table_t* slf, value_t key);
