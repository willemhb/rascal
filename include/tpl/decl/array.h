#ifndef rascal_tpl_array_h
#define rascal_tpl_array_h

#include "common.h"
#include "tpl/type.h"

/* Generic safe array types (inspired by sds).

   Represented as a normal array of type X, with a header
   hidden before the first element of the array. */

#define ARRAY(ARR, X)						\
  /* define array as pointer to elements */			\
  typedef X *TYPE(ARR);						\
								\
  typedef struct						\
  {									\
    size_t len;								\
    size_t cap;								\
    X elements[];							\
  } HEADER(ARR);							\
  									\
  TYPE(ARR)   make_##ARR( size_t n, X *ini );				\
  void        resize_##ARR( TYPE(ARR) *ARR, size_t new_len );		\
  void        trim_##ARR( TYPE(ARR) *ARR );				\
  TYPE(ARR)   dup_##ARR( TYPE(ARR) ARR );				\
  void        free_##ARR( TYPE(ARR) ARR );				\
  HEADER(ARR) *ARR##_header( TYPE(ARR) ARR );				\
  size_t      ARR##_len( TYPE(ARR) ARR );				\
  size_t      ARR##_cap( TYPE(ARR) ARR );				\
  size_t      ARR##_size( TYPE(ARR) ARR );				\
  size_t      ARR##_alloc( TYPE(ARR) ARR );				\
  size_t      ARR##_cpy( TYPE(ARR) ARR, X *src, size_t n );		\
  void        ARR##_fill( TYPE(ARR), X x );				\
  size_t      ARR##_push( TYPE(ARR) *ARR, X x );			\
  X           ARR##_pop( TYPE(ARR) *ARR );				\
  size_t      ARR##_pushn( TYPE(ARR) *ARR, size_t n, ... );		\
  X           ARR##_popn( TYPE(ARR) *ARR, size_t n );			\
  X	     *ARR##_at( TYPE(ARR) ARR, long i );		        \
  X           ARR##_ref( TYPE(ARR) ARR, long i );                       \
  X           ARR##_set( TYPE(ARR) ARR, long i, X x );			\
  X           ARR##_swap( TYPE(ARR) ARR, long i, X x )

#endif
