#include <stdlib.h>
#include <string.h>

#include "vm/memory.h"

#include "util/hset.h"

/* commentary */

/* C types */

/* globals */

/* API */
hset_t *make_hset( size_t n_keys, pad_array_size_fn_t padfn )
{
  hset_t *out = alloc(sizeof(hset_t));

  if ( padfn != NULL )
    init_hset(out, n_keys, padfn);

  return out;
}

void init_hset( hset_t *hset, size_t n_keys, pad_array_size_fn_t padfn )
{
  hset->len   = 0;
  hset->cap   = padfn(n_keys, 0, 0);
  hset->table = alloc_array(hset->cap, sizeof(void*));
}

void free_hset( hset_t *hset )
{
  dealloc_array(hset->table, hset->len, sizeof(void*));
  dealloc(hset, sizeof(hset_t));
}

void clear_hset( hset_t *hset, pad_array_size_fn_t padfn )
{
  hset->cap          = padfn(0, 0, hset->cap);
  free(hset->table);
  hset->table        = alloc_array(hset->cap, sizeof(void*));
}
