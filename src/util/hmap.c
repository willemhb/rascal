#include <stdlib.h>
#include <string.h>

#include "memory.h"

#include "util/collection.h"
#include "util/hmap.h"

/* commentary */

/* C types */

/* globals */

/* API */
hmap_t *make_hmap( void )
{
  return alloc(sizeof(hmap_t));
}

void init_hmap( hmap_t *hmap, size_t n_keys, pad_fn_t padfn )
{
  hmap->len    = 0;
  hmap->cap    = padfn(n_keys, 0, 0);
  hmap->table  = alloc_array(hmap->cap, sizeof(void*)*2);
}

void free_hmap( hmap_t *hmap )
{
  dealloc_array(hmap->table, hmap->cap, sizeof(void*)*2);
  dealloc(hmap, sizeof(hmap_t));
}

void clear_hmap( hmap_t *hmap, pad_fn_t padfn )
{
  hmap->cap          = padfn(0, 0, hmap->cap);
  free(hmap->table);
  hmap->table        = alloc_array(hmap->cap, sizeof(void*)*2);
}
