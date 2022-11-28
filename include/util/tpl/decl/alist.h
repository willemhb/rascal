#ifndef rl_util_decl_alist_h
#define rl_util_decl_alist_h

#include "common.h"
#include "util/tpl/impl/type.h"

#define ALIST( X )					\
  struct						\
  {							\
    size_t  len;					\
    size_t  cap;					\
    X      *elements;					\
  }

#define ALSIZE( X ) sizeof(ALIST(X))

#define MAKE_ALIST( A, X )			\
  TYPE(A) *make_##A( size_t len )

#define RESIZE_ALIST( A, X )			\
  void resize_##A( TYPE(A) *A, size_t new_len )

#define FREE_ALIST( A, X )			\
  void free_##A( TYPE(A) *alist )

#define ALIST_PUSH( A, X )			\
  void A##_push( TYPE(A) *A, X elt )

#define ALIST_POP( A, X )			\
  X A##_pop( TYPE(A) *A )

#define ALIST_PUSHN( A, X )			\
  void A##_pushn( TYPE(A) *A, size_t n, ... )

#define ALIST_POPN( A, X )			\
  X A##_popn( TYPE(A) *A, size_t n )

#define WRITETO_ALIST( A, X )				\
  size_t writeto_##A( TYPE(A) *dst, X *src, size_t n )

#define READFROM_ALIST( A, X )				\
  size_t readfrom_##A( X *dst, TYPE(A) *src, size_t n )

#define ALIST_REF( A, X )			\
  X A##_ref( TYPE(A) *A, long i )

#define ALIST_SET( A, X )			\
  X A##_set( TYPE(A) *A, long i, X x )

#define ALIST_SWAP( A, X )			\
  X A##_swap( TYPE(A) *A, long i, X x )

#endif
