#ifndef rl_util_hset_h
#define rl_util_hset_h

#include "common.h"

/* commentary */

/* C types */
typedef struct
{
  size_t   len;
  size_t   cap;
  void   **table;
} hset_t;

/* globals */

/* API */
hset_t *make_hset( size_t n_keys, funcptr padfn );
void    free_hset( hset_t *hset );
void    clear_hset( hset_t *hset, funcptr padfn );

/* runtime */

/* convenience */
#define hset_cell( set, i )			\
  ((_Generic((set),				\
	     void**:(void**)(set),		\
	     hset_t*:((hset_t*)(set))->table	\
	     ))+(i))

#define hset_member( hset, i, M )   (*(M*)(hset_cell(hset, i)))

#endif
