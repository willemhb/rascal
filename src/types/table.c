#include <string.h>

#include "types/table.h"

#include "lang/hash.h"
#include "lang/compare.h"

#include "runtime/memory.h"

#include "util/collections.h"

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
      table_put(self, args[i], args[i+1]);

  self->init = true;
}

void resize_table(Table* self, uint n) {
  uint c = pad_table_size(n, self->cap);

  if (c != self->cap)
    rehash_table(self, c);
}

