#include <stdlib.h>
#include <string.h>

#include "vm/memory.h"

#include "util/collection.h"
#include "util/hset.h"


/* commentary */

/* C types */

/* globals */

/* API */
hset_t *make_hset( size_t n_keys, funcptr padfn )
{
  size_t cap  = ((size_t (*)(size_t, size_t, size_t))padfn)(n_keys, 0, 0);
  hset_t *out = alloc(sizeof(hset_t));
  out->len    = 0;
  out->cap    = cap;
  out->table  = alloc_array(cap, sizeof(void*));

  return out;
}

void free_hset( hset_t *hset )
{
  dealloc_array(hset->table, hset->len, sizeof(void*));
  dealloc(hset, sizeof(hset_t));
}

void clear_hset( hset_t *hset, funcptr padfn )
{
  hset->cap          = ((size_t (*)(size_t, size_t, size_t))padfn)(0, 0, hset->cap);
  free(hset->table);
  hset->table        = alloc_array(hset->cap, sizeof(void*));
}
