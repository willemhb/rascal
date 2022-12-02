#include <stdlib.h>
#include <string.h>

#include "vm/memory.h"

#include "util/collection.h"
#include "util/hmap.h"


/* commentary */

/* C types */

/* globals */

/* API */
hmap_t *make_hmap( size_t n_keys, pad_array_size_fn_t padfn )
{
  
  hmap_t *out = alloc(sizeof(hmap_t));

  if ( padfn != NULL )
    init_hmap(out, n_keys, padfn);
  
  return out;
}

void init_hmap( hmap_t *hmap, size_t n_keys, pad_array_size_fn_t padfn )
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

void clear_hmap( hmap_t *hmap, pad_array_size_fn_t padfn )
{
  hmap->cap          = padfn(0, 0, hmap->cap);
  free(hmap->table);
  hmap->table        = alloc_array(hmap->cap, sizeof(void*)*2);
}
