#ifndef rl_util_alist_h
#define rl_util_alist_h

#include "rascal.h"

/* commentary

   Generic alist type.

   See util/tpl/?/alist.h for specialization macros. */

/* C types */
typedef struct alist_t
{
  size_t  len;
  size_t  cap;
  void  **elements;
} alist_t;

/* API */

alist_t *make_alist( size_t len, pad_array_size_fn_t padfn );
void     resize_alist( alist_t *alist, size_t new_len, pad_array_size_fn_t padfn );
void     reset_alist( alist_t *alist, pad_array_size_fn_t padfn );
void     free_alist( alist_t *alist );
void     mark_object_alist( alist_t *alist );
void     mark_value_alist( alist_t *alist );

/* runtime */

/* convenience */
#define alist_member(alist, i, X)				\
  (*(X*)(_Generic((alist),					\
		  alist_t*:((alist_t*)(alist))->elements,	\
		  default:(typeof(alist))(alist))+(i)))

#define alist_at(alist, i, X)					\
  ((X*)(_Generic((alist),					\
		 alist_t*:((alist_t*)((alist)))->elements,	\
		 default:(typeof(alist))(alist))+(i)))

#endif
