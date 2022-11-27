#ifndef rascal_tpl_array_h
#define rascal_tpl_array_h

#include "tpl/impl/type.h"

/* common array header */
#define ARRAY( X )				\
  struct array_t arr;				\
  X data[]

#define ARRHEAD struct array_t arr

#define MAKE_ARR( A, X )			\
  TYPE(A) make_##A( size_t n, X *ini )

#define FREE_ARR( A, X )			\
  void free_##A( TYPE(A) A )

#define RESIZE_ARR( A, X )				\
  size_t resize_##A( TYPE(A) A, size_t new_count )

#define ARRAY_REF( A, X )			\
  X A##_ref( TYPE(A) A, long i )

#define ARRAY_SET( A, X )			\
  X A##_set( TYPE(A), long i, X x )

#define ARRAY_ADD( A, X )			\
  size_t A##_add( TYPE(A) A, size_t n, ... )

#define ARRAY_POP( A, X )			\
  X A##_pop( TYPE(A) A, size_t n )

#endif
