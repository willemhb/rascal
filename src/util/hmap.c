#include <stdlib.h>
#include <string.h>

#include "vm/memory.h"

#include "util/collection.h"
#include "util/hmap.h"


/* commentary */

/* C types */

/* globals */

/* API */
hmap_t *make_hmap( size_t n_keys, funcptr padfn )
{
  
  size_t cap  = ((size_t(*)(size_t, size_t, size_t))padfn)(n_keys, 0, 0);
  hmap_t *out = alloc(sizeof(hmap_t));
  out->len    = 0;
  out->cap    = cap;
  out->table  = alloc_array(cap, sizeof(void*)*2);

  return out;
}

void free_hmap( hmap_t *hmap )
{
  dealloc_array(hmap->table, hmap->cap, sizeof(void*)*2);
  dealloc(hmap, sizeof(hmap_t));
}

void clear_hmap( hmap_t *hmap, funcptr padfn )
{
  hmap->cap          = ((size_t(*)(size_t, size_t, size_t))padfn)(0, 0, hmap->cap);
  free(hmap->table);
  hmap->table        = alloc_array(hmap->cap, sizeof(void*)*2);
}
