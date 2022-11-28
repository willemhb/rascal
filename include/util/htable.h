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
  void   **table;
} htable_t;

/* globals */

/* API */
htable_t *make_htable( size_t n_keys );
void      free_htable( htable_t *htable );
void      resize_htable( htable_t *htable, size_t new_n_keys );
void      reset_htable( htable_t *htable );

/* runtime */

/* convenience */

#endif
