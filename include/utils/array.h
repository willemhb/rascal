#ifndef rascal_array_h
#define rascal_array_h

#include "common.h"

// macros, globals, common array utilities, and array types that don't fit elsewhere
#define resize_vec(p, o, n, t) resize((p), (o)*sizeof(t), (n)*sizeof(t))
#define alloc_vec(n, t)        alloc((n)*sizeof(t))
#define dealloc_vec(p, n, t)   dealloc((p), (n)*sizeof(t))
#define duplicate_vec(p, n, t) duplicate_bytes((p), (n) * sizeof(t))

// array describe macros
#define ARRAY_SLOTS(V, F, f)						\
  F        f;								\
  arity_t  len;								\
  V       *data

#define INIT_ARRAY(V, F, f)

// forward declarations
arity_t pad_alist_size( arity_t newl, arity_t oldl, arity_t oldc, arity_t minc );
arity_t pad_stack_size( arity_t newl, arity_t oldl, arity_t oldc, arity_t minc );

#endif
