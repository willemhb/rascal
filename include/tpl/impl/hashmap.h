#ifndef rl_tpl_impl_hashmap_h
#define rl_tpl_impl_hashmap_h

#include <stdlib.h>

#include "util/hashing.h"
#include "util/collection.h"
#include "util/memory.h"

#include "tpl/type.h"

#undef HASHMAP

#define HASHMAP( HM, K, V, padfn, hashfn, cmpfn, nokey, noval )		\
  hmap_t *make_##HM( size_t n_keys, void **ini )			\
  {									\
    hmap_t *hmap = make_hmap(n_keys, padfn);				\
    /* initialize empty cells */					\
    init_##HM(hmap);							\
    if ( ini )								\
      {									\
	for ( size_t i=0; i<n_keys; i++ )				\
	  {								\
	    K key = hmap_key(ini, i, K);				\
	    V val = hmap_val(ini, i, V);				\
	    HM##_put(hmap, key, val);					\
	  }								\
      }									\
    return hmap;							\
  }									\
  									\
  void init_##HM( hmap_t *hmap )					\
  {									\
    init_##HM##_table(hmap->table, hmap->cap);				\
  }									\
  									\
  void init_##HM##_table( void **table, size_t cap )			\
  {									\
    for ( size_t i=0; i<cap; i++ )					\
      {									\
	hmap_key(table, i, K) = nokey;					\
	hmap_val(table, i, V) = noval;					\
      }									\
  }									\
  									\
  size_t resize_##HM( hmap_t *hmap, size_t new_n_keys )			\
  {									\
    size_t new_cap = padfn(new_n_keys, hmap->len, hmap->cap);		\
    if ( new_cap != hmap->cap )						\
      {									\
	void **table = calloc_s(hmap->cap, sizeof(void*)*2);		\
	rehash_##HM(hmap, table, new_cap);				\
	free(hmap->table);						\
	hmap->table      = table;					\
	hmap->cap        = new_cap;					\
      }									\
    hmap->len = new_n_keys;						\
    return hmap->len;							\
  }									\
									\
  void rehash_##HM( hmap_t *hmap, void **table, size_t new_cap )	\
  {									\
    init_##HM##_table(table, new_cap);					\
    									\
    for ( size_t i=0, n=0; i<hmap->cap && n < hmap->len; i++)		\
      {									\
	K key = hmap_key(hmap, i, K);					\
	if ( key == nokey )						\
	  {								\
	    continue;							\
	  }								\
	V val = hmap_val(hmap, i, V);					\
	ulong hash = hashfn(key);					\
	ulong idx  = hash & (new_cap-1);				\
	while ( hmap_key(table, idx, K) != nokey )			\
	  {								\
	    idx = (idx+1) & (new_cap-1);				\
	  }								\
	hmap_key(table, idx, K) = key;					\
	hmap_val(table, idx, V) = val;					\
	n++;								\
      }									\
  }									\
  									\
  void free_##HM( hmap_t *hmap ) { free_hmap(hmap); }			\
  									\
  void clear_##HM( hmap_t *hmap )					\
  {									\
    clear_hmap(hmap, padfn);						\
    init_##HM(hmap);							\
  }									\
									\
  void **HM##_locate( hmap_t *hmap, K key )				\
  {									\
    ulong hash  = hashfn(key);						\
    ulong index = hash & (hmap->cap-1);					\
    K hkey;								\
    while ( (hkey=hmap_key(hmap, index, K)) != nokey )			\
      {									\
	if ( cmpfn(hkey, key) )						\
	  break;							\
	index = (index+1) & (hmap->cap-1);				\
      }									\
    return hmap_cell(hmap, index);					\
  }									\
  									\
  V HM##_get( hmap_t *hmap, K key )					\
  {									\
    void **cell = HM##_locate(hmap, key);				\
    return hmap_val(cell, 0, V);					\
  }									\
  									\
  V HM##_set( hmap_t *hmap, K key, V val )				\
  {									\
    void **cell = HM##_locate(hmap, key);				\
    if ( hmap_key(cell, 0, K) == nokey )				\
      return noval;							\
    hmap_val(cell, 0, V) = val;						\
    return val;								\
  }									\
  									\
  V HM##_put( hmap_t *hmap, K key, V val )				\
  {									\
    void **cell = HM##_locate(hmap, key);				\
    if ( *(K*)cell == nokey )						\
      {									\
	hmap_key(cell, 0, K) = key;					\
	hmap_val(cell, 0, V) = val;					\
	resize_##HM(hmap, hmap->len+1);					\
      }									\
    else								\
      {									\
	hmap_val(cell, 0, V) = val;					\
      }									\
    return val;								\
  }									\
  									\
  V HM##_intern( hmap_t *hmap, K key, V val )				\
  {									\
    void **cell = HM##_locate(hmap, key);				\
    if ( *(K*)cell == nokey )						\
      {									\
	hmap_key(cell, 0, K) = key;					\
	hmap_val(cell, 0, V) = val;					\
	resize_##HM(hmap, hmap->len+1);					\
      }									\
    else								\
      {									\
	val = hmap_val(cell, 0, V);					\
      }									\
    return val;								\
  }									\
  									\
  bool HM##_add( hmap_t *hmap, K key, V val )				\
  {									\
    void **cell = HM##_locate(hmap, key);				\
    if (*(K*)cell == nokey )						\
      {									\
	hmap_key(cell, 0, K) = key;					\
	hmap_val(cell, 0, V) = val;					\
	resize_##HM(hmap, hmap->len+1);					\
	return true;							\
      }									\
    return false;							\
  }									\
  									\
  bool HM##_has( hmap_t *hmap, K key )					\
  {									\
    void **cell = HM##_locate(hmap, key);				\
    return *(K*)cell != nokey;						\
  }									\
  									\
  bool HM##_remove( hmap_t *hmap, K key )				\
  {									\
    void **cell = HM##_locate(hmap, key);				\
    if (*(K*)cell == nokey )						\
      return false;							\
    hmap_key(cell, 0, K) = nokey;					\
    hmap_val(cell, 0, V) = noval;					\
    resize_##HM(hmap, hmap->len-1);					\
    return true;							\
  }

#endif
