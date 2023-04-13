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
  entries    = allocate(newt);

  if (newt > oldt)
    init_entries(entries, oldc, newc);

  return entries;
}

static void rehash_table(table_t* slf) {
  
#define REHASH_TABLE(o)                                     \
  do {                                                      \
    __auto_type __o = o;                                    \
    usize mask      = slf->cap-1;                           \
                                                            \
    for (usize i=0; i < slf->cnt;) {                        \
      value_t key = slf->entries[i*2];                      \
      uhash hash  = rl_hash(key);                           \
      usize idx   = hash & mask;                            \
                                                            \
      while (__o[idx] > -1) {                               \
        idx = (idx+1) & mask;                               \
      }                                                     \
      __o[idx] = i;                                         \
    }                                                       \
 } while (0)

  if (slf->cap <= INT8_MAX)
    REHASH_TABLE(slf->o8);

  else if (slf->cap <= INT16_MAX)
    REHASH_TABLE(slf->o16);

  else if (slf->cap <= INT32_MAX)
    REHASH_TABLE(slf->o32);

  else
    REHASH_TABLE(slf->o64);
  
#undef REHASH_TABLE
}

void reset_table(table_t* slf) {
  deallocate_ords(slf->ords, slf->cap);
  deallocate_entries(slf->entries, slf->cap);
  init_table(slf);
}

void init_table(table_t* slf) {
  slf->cnt     = 0;
  slf->cap     = MINC;
  slf->ords    = allocate_ords(slf->cap);
  slf->entries = allocate_entries(slf->cap);
}

bool resize_table(table_t* slf, usize n) {
  usize lim = ceil(n * NLOADF);

  slf->cnt = n;

  if (slf->cap == MINC)
    return false;

  if (lim <= slf->cap && lim >= (slf->cap >> 1))
    return false;

  usize newc   = ceil2(lim+1);
  usize oldc   = slf->cap;
  slf->ords    = reallocate_ords(slf->ords, oldc, newc);
  slf->entries = reallocate_entries(slf->entries, oldc, newc);
  slf->cap     = newc;

  return true;
}

long table_find(table_t* slf, value_t key) {
  uhash h   = rl_hash(key);
  usize m   = slf->cap - 1;
  usize idx = h & m;
  long  i   = -1;

#define TABLE_FIND(o)                           \
  do {                                          \
    __auto_type __o = o;                        \
    while ((i=__o[idx]) != -1) {                \
      if (rl_equal(key, slf->entries[i*2]))     \
        break;                                  \
      idx = (idx+1) & m;                        \
    }                                           \
  } while (0)

  if (slf->cap < INT8_MAX)
    TABLE_FIND(slf->o8);

  else if (slf->cap < INT16_MAX)
    TABLE_FIND(slf->o16);
  
  else if (slf->cap < INT32_MAX)
    TABLE_FIND(slf->o32);

  else
    TABLE_FIND(slf->o64);
  
  return i;
#undef TABLE_FIND
}

value_t table_get(table_t* slf, value_t key) {
  long i = table_find(slf, key);

  if (i == -1)
    return NOTFOUND;

  return slf->entries[i*2+1];
}

value_t table_set(table_t* slf, value_t key, value_t val) {
  long i = table_find(slf, key);

  if (i == -1) {
    i = slf->cnt;
    slf->entries[i*2] = key;

    if (resize_table(slf, i+1))
      rehash_table(slf);
  }

  value_t out = slf->entries[i*2+1];
  slf->entries[i*2+1] = val;

  return out;
}

bool table_add(table_t* slf, value_t key, value_t val) {
  long i = table_find(slf, key);
  bool out = i == -1;

  if (out) {
    i = slf->cnt;
    slf->entries[i*2] = key;
    slf->entries[i*2+1] = val;

    if (resize_table(slf, i+1))
      rehash_table(slf);
  }

  return out;
}
