#include <string.h>
#include <assert.h>

#include "types/table.h"

#include "lang/hash.h"
#include "lang/compare.h"

#include "runtime/memory.h"

#include "util/collections.h"
#include "util/hash.h"

/* local helpers */
static void rehash_table(Table* self, uint newc) {
  int* newo = callocate(newc, sizeof(int), -1), *oldo = self->ord;
  uint mask = (newc-1), oldc = self->cap;

  for (uint i=0, n=0; i < self->cap && n < self->count; ) {
    if (oldo[i] < 0)
      continue;

    uhash h  = hash(self->table[i].key, self->eql);
    uint idx = h & mask;

    while (newo[idx] > 0)
      idx = (idx+1) & mask;

    newo[idx] = oldo[i];
  }

  cdeallocate(self->ord, self->cap, sizeof(int));
  self->ord   = newo;
  self->table = creallocate(self->table, newc*2, oldc*2, sizeof(Val), NOTFOUND);
  self->cap   = newc;
}

/* API */
Table* new_table(bool eql, uint n, Val* args) {
  Table* out = construct(TABLE_TYPE, 1, 0);
  init_table(out, eql, n, args);
  return out;
}

void init_table(Table* self, bool eql, uint n, Val* args) {
  init_obj((Obj*)self, TABLE_TYPE);

  self->eql    = eql;
  self->count  = 0;
  self->cap    = pad_table_size(n, 0);
  self->ord    = callocate(self->cap, sizeof(int), -1);
  self->table  = callocate(self->cap*2, sizeof(Val), NOTFOUND);

  if (args)
    for (uint i=0; i<n*2; i+= 2)
      table_set(self, args[i], args[i+1]);

  self->init = true;
}

void resize_table(Table* self, uint n) {
  uint c = pad_table_size(n, self->cap);

  if (c != self->cap)
    rehash_table(self, c);
}

int* table_lookup(Table* self, Val key) {
  uhash h = hash(key, self->eql);
  uint  m = self->cap-1;
  uint  i = h & m;
  bool (*cmp)(Val x, Val y) = self->eql ? equal : same;
  int *o;

  while (*(o=self->ord+i) > 0) {
    if (cmp(key, self->table[*o].key))
      break;

    i = (i+1) & m;
  }

  return o;
}

Val* table_nth(Table* self, uint n) {
  assert(n <= self->count);
  return &self->table[n].key;
}

Val table_get(Table* self, Val key) {
  int* o = table_lookup(self, key);

  if (*o == -1)
    return NOTFOUND;

  return table_nth(self, *o)[1];
}

Val table_set(Table* self, Val key, Val val) {
  if (self->init)
    resize_table(self, self->count+1);

  bool added = false;
  int *o     = table_lookup(self, key);

  if (*o == -1) {
    *o = self->count++;
    added    = true;
  }

  Val* spc = table_nth(self, *o);
  spc[1]   = val;

  if (added)
    spc[0] = key;

  return val;
}

Val table_del(Table* self, Val key) {
  int *o = table_lookup(self, key);

  if (*o == -1)
    return NOTFOUND;

  Val* spc = table_nth(self, *o);
  Val  out = spc[1];
  spc[0]   = NOTFOUND;
  spc[1]   = NOTFOUND;

  resize_table(self, --self->count);

  return out;
}

// initialization -------------------------------------------------------------
void table_init(void) {
  // initialize types ---------------------------------------------------------
  extern void  trace_table(void* self);
  extern usize destruct_table(void* self);
  extern void  print_table(Val x, void* state);
  extern uhash hash_table(Val x, void* state);
  extern bool  equal_tables(Val x, Val y, void* state);
  extern int   compare_tables(Val x, Val y, void* state);

  MetaTables[TABLE_TYPE] = (Mtable) {
    .name     =intern("table"),
    .type     =TABLE_TYPE,
    .kind     =DATA_KIND,
    .type_hash=hash_uint(TABLE_TYPE),
    .size     =sizeof(Table),

    .trace    =trace_table,
    .destruct =destruct_table,
    .print    =print_table,
    .hash     =hash_table,
    .equal    =equal_tables,
    .compare  =compare_tables
  };
}
