#include <stdlib.h>
#include <string.h>

#include "util/collection.h"
#include "util/memory.h"
#include "util/htable.h"


/* commentary */

/* C types */

/* globals */

/* API */
void rehash_htable( htable_t *htable, long *new_ord, size_t new_cap )
{
  long  *old_ord   = htable->order;

  for (size_t i=0, n=0; i<htable->cap && n <htable->len; i++)
    {
      if ( old_ord[i*2] == -1 )
	continue;

      long order  = old_ord[i*2];
      ulong hash  = old_ord[i*2+1];
      ulong index = hash & (new_cap-1);

      while ( new_ord[index*2] > -1 )
	index = (index+1) & (new_cap-1);
      
      new_ord[index*2]   = order;
      new_ord[index*2+1] = hash;
    }
}

void free_htable( htable_t *htable )
{
  free(htable->order);
  free(htable->table);
  free(htable);
}

static long *new_orders( size_t cap );

hashmap_t *make_hashmap( size_t n_keys )
{
  size_t cap = pad_table_size(n_keys, 0);

  hashmap_t *out = malloc_s(sizeof(hashmap_t));
  out->len = 0;
  out->cap = cap;
  out->order = new_orders(cap);
  out->table = calloc_s(cap, sizeof(void*)*2);

  return out;
}

void resize_hashmap( hashmap_t *hmap, size_t new_n_keys )
{
  size_t new_cap = pad_table_size(new_n_keys, hmap->cap);

  if ( new_cap != hmap->cap )
    {
      long *new_ord = new_orders(new_cap);

      rehash_hashmap(hmap, new_ord, new_cap);
      free(hmap->order);

      hmap->order = new_ord;
      hmap->table = crealloc_s(hmap->table, new_cap, sizeof(void*)*2);
      hmap->cap   = new_cap;
    }

  hmap->len = new_n_keys;
}

void rehash_hashmap( hashmap_t *hmap, long *new_ord, size_t new_cap )
{
  rehash_htable(hmap, new_ord, new_cap);
}

void free_hashmap( hashmap_t *hmap )
{
  free_htable(hmap);
}

hashset_t *make_hashset( size_t n_keys )
{
  size_t cap = pad_table_size(n_keys, 0);

  hashset_t *out = malloc_s(sizeof(hashset_t));

  out->len = 0;
  out->cap = cap;
  out->order = new_orders(cap);
  out->table = calloc_s(cap, sizeof(void*));

  return out;
}

void resize_hashset( hashset_t *hset, size_t new_n_keys )
{
  size_t new_cap = pad_table_size(new_n_keys, hset->cap);

  if ( new_cap != hset->cap )
    {
      long *new_ord = new_orders(new_cap);

      rehash_hashset(hset, new_ord, new_cap);
      free(hset->order);

      hset->order = new_ord;
      hset->table = crealloc_s(hset->table, new_cap, sizeof(void*));
      hset->cap   = new_cap;
    }

  hset->len = new_n_keys; 
}

void rehash_hashset( hashset_t *hset, long *new_ord, size_t new_cap )
{
  rehash_htable(hset, new_ord, new_cap);
}

void free_hashset( hashset_t *hset )
{
  free_htable(hset);
}

/* runtime */

/* convenience */
static long *new_orders( size_t n )
{
  long *out = calloc_s(n, sizeof(long)*2);

  memset(out, 255, n*sizeof(long)*2);

  return out;
}

