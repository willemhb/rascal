#ifndef rl_util_htable_h
#define rl_util_htable_h

#include "common.h"

/* commentary

   A basic generic hash table type.

   See tpl/?/table.h for implementation macros. */

/* C types */
typedef struct
{
  size_t   len;
  size_t   cap;
  long    *order; /* saved hash and insertion order */
  void   **table; /* actual key/value pairs */
} htable_t;

typedef htable_t hashmap_t;
typedef htable_t hashset_t;

/* globals */

/* API */
void rehash_htable( htable_t *htable, long *new_ord, size_t new_cap );
void free_htable( htable_t *htable );

hashmap_t *make_hashmap( size_t n_keys );
void       resize_hashmap( hashmap_t *hmap, size_t new_n_keys );
void       rehash_hashmap( hashmap_t *hmap, long *new_ord, size_t new_cap );
void       free_hashmap( hashmap_t *hmap );

hashset_t *make_hashset( size_t n_keys );
void       resize_hashset( hashset_t *hset, size_t new_n_keys );
void       rehash_hashset( hashset_t *hset, long *new_ord, size_t new_cap );
void       free_hashset( htable_t *hashset );

/* runtime */

/* convenience */

#endif
