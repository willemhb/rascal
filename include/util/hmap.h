#ifndef rl_util_hmap_h
#define rl_util_hmap_h

#include "rascal.h"

#include "util/collection.h"

/* commentary

   A basic generic hash table type.

   See tpl/?/table.h for implementation macros. */

/* C types */
typedef struct
{
  size_t   len;
  size_t   cap;
  void   **table; /* key/value pairs */
} hmap_t;

/* globals */

/* API */
hmap_t *make_hmap( void );
void    init_hmap( hmap_t *hmap, size_t n_keys, pad_array_size_fn_t padfn );
void    free_hmap( hmap_t *hmap );
void    clear_hmap( hmap_t *hmap, pad_array_size_fn_t padfn );

/* runtime */

/* convenience */
#define hmap_cell( map, i )			\
  ((_Generic((map),				\
	     void**:(void**)(map),		\
	     hmap_t*:((hmap_t*)(map))->table	\
	     ))+(i)*2)

#define hmap_key( hmap, i, K )      (*(K*)hmap_cell(hmap, i))
#define hmap_val( hmap, i, V )      (*(V*)(hmap_cell(hmap, i)+1))

#endif
