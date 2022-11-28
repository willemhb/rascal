#ifndef rl_util_tpl_impl_alist_h
#define rl_util_tpl_impl_alist_h

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

#include "util/tpl/impl/type.h"
#include "util/tpl/undef/alist.h"

#include "util/number.h"
#include "util/collection.h"
#include "util/memory.h"

#define MAKE_ALIST( A, X )			\
  TYPE(A) *make_##A( size_t len )		\
  {						\
    size_t cap  = pad_alist_size(len, 0);	\
    TYPE(A)* A  = malloc_s(ALSIZE(X));		\
    A->len      = len;				\
    A->cap      = cap;				\
    A->elements = calloc_s(cap, sizeof(X));	\
    return A;					\
  }

#define RESIZE_ALIST( A, X )						\
  void resize_##A( TYPE(A) *A, size_t new_len )				\
  {									\
    size_t new_cap = pad_alist_size(new_len, A->cap);			\
									\
    if ( new_cap != A->cap )						\
      A->elements = crealloc_s(A->elements, new_cap, sizeof(X));	\
    									\
    A->len = new_len;							\
    A->cap = new_cap;							\
  }

#define FREE_ALIST( A, X )			\
  void free_##A( TYPE(A) *A )			\
  {						\
    free(A->elements);				\
    free(A);					\
  }

#define ALIST_PUSH( A, X )			\
  void A##_push( TYPE(A) *A, X x )		\
  {						\
    resize_##A(A, A->len+1);			\
    A->elements[A->len-1] = x;			\
  }

#define ALIST_POP( A, X )			\
  X A##_pop( TYPE(A) *A )			\
  {						\
    X x = A->elements[A->len-1];		\
    resize_##A(A, A->len-1);			\
    return x;					\
  }

#define ALIST_PUSHN( A, X, V )			\
  void A##_pushn( TYPE(A) *A, size_t n, ... )	\
  {						\
    size_t old_len = A->len;			\
    resize_##A(A, A->len+n);			\
    X *cell = A->elements+old_len-1;		\
    va_list va; va_start(va, n);		\
    for (size_t i=0; i<n; i++)			\
      *(cell++) = va_arg(va, V);		\
    va_end(va);					\
  }

#define ALIST_POPN( A, X )			\
  X A##_popn( TYPE(A) *A, size_t n )		\
  {						\
    X x = A->elements[A->len-1];		\
    resize_##A(A, A->len-n);			\
    return x;					\
  }

#define WRITETO_ALIST( A, X )				\
  size_t writeto_##A( TYPE(A) *A, X *src, size_t n )	\
  {							\
    size_t old_len = A->len;				\
    resize_##A(A, A->len+n);				\
    X *buf = A->elements+old_len;			\
    memcpy(buf, src, n*sizeof(X));			\
    return A->len;					\
  }

#define READFROM_ALIST( A, X )				\
  size_t readfrom_##A( X *dst, TYPE(A) *A, size_t n )	\
  {							\
    n = min(n, A->len);					\
    memcpy(dst, A->elements, n * sizeof(X));		\
    return n;						\
  }

#define ALIST_REF( A, X )			\
  X A##_ref( TYPE(A) *A, long i )		\
  {						\
    if ( i < 0 )				\
      i += A->len;				\
    assert(i > 0 && (size_t)i < A->len);	\
    return A->elements[i];			\
  }

#define ALIST_SET( A, X )			\
  X A##_set( TYPE(A) *A, long i, X x )		\
  {						\
    if ( i < 0 )				\
      i += A->len;				\
    assert(i > 0 && (size_t)i < A->len);	\
    A->elements[i] = x;				\
    return x;					\
  }

#define ALIST_SWAP( A, X )			\
  X A##_swap( TYPE(A) *A, long i, X x )		\
  {						\
    if ( i < 0 )				\
      i += A->len;				\
    assert(i > 0 && (size_t)i < A->len);	\
    X out = A->elements[i];			\
    A->elements[i] = x;				\
    return out;					\
  }

#endif
