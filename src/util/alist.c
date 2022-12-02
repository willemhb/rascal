#include <stdlib.h>

#include "util/alist.h"
#include "vm/memory.h"

/* commentary */

/* C types */

/* globals */

/* API */
alist_t *make_alist( size_t len, pad_array_size_fn_t padfn )
{
  size_t   cap = ((size_t(*)(size_t, size_t, size_t))padfn)(len, 0, 0);
  alist_t *out = alloc(sizeof(alist_t));
  
  out->len = len;
  out->cap = cap;
  out->elements = alloc_array(cap, sizeof(void*));

  return out;
}

void resize_alist( alist_t *alist, size_t new_len, pad_array_size_fn_t padfn )
{
  size_t new_cap = ((size_t(*)(size_t, size_t, size_t))padfn)(new_len, alist->len, alist->cap);

  if ( new_cap != alist->cap )
    {
      alist->elements = adjust_array(alist->elements, alist->cap, new_cap, sizeof(void*));
      alist->cap = new_cap;
    }
  alist->len = new_len;
}

void reset_alist( alist_t *alist, pad_array_size_fn_t padfn )
{
  resize_alist(alist, 0, padfn);
}

void free_alist( alist_t *alist )
{
  dealloc_array(alist->elements, alist->cap, sizeof(void*));
  dealloc(alist, sizeof(alist_t));
}

/* runtime */

/* convenience */
