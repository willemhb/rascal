#ifndef rl_util_tpl_impl_buffer_h
#define rl_util_tpl_impl_buffer_h

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "util/tpl/impl/type.h"
#include "util/tpl/undef/buffer.h"

#include "util/number.h"
#include "util/collection.h"
#include "util/memory.h"

#define MAKE_BUFFER( B, X )			\
  TYPE(B) *make_##B( size_t len )		\
  {						\
    size_t cap  = pad_buffer_size(len, 0);	\
    TYPE(B)* B  = malloc_s(BUFSIZE(X));		\
    B->len      = len;				\
    B->cap      = cap;				\
    B->elements = calloc_s(cap, sizeof(X));	\
    return B;					\
  }

#define RESIZE_BUFFER( B, X )						\
  void resize_##B( TYPE(B) *B, size_t new_len )				\
  {									\
    size_t new_cap = pad_buffer_size(new_len, B->cap);			\
									\
    if ( new_cap != B->cap )						\
      B->elements = crealloc_s(B->elements, new_cap, sizeof(X));	\
    									\
    B->len = new_len;							\
    B->cap = new_cap;							\
  }

#define FREE_BUFFER( B, X )			\
  void free_##B( TYPE(B) *B )			\
  {						\
    free(B->elements);				\
    free(B);					\
  }

#define BUFFER_PUSH( B, X )			\
  void B##_push( TYPE(B) *B, X x )		\
  {						\
    resize_##B(B, B->len+1);			\
    B->elements[B->len-1] = x;			\
  }

#define BUFFER_POP( B, X )			\
  X B##_pop( TYPE(B) *B )			\
  {						\
    X x = B->elements[B->len-1];		\
    resize_##B(B, B->len-1);			\
    return x;					\
  }

#define BUFFER_PUSHN( B, X, V )			\
  void B##_pushn( TYPE(B) *B, size_t n, ... )	\
  {						\
    size_t old_len = B->len;			\
    resize_##B(B, B->len+n);			\
    X *cell = B->elements+old_len-1;		\
    va_list va; va_start(va, n);		\
    for (size_t i=0; i<n; i++)			\
      *(cell++) = va_arg(va, V);		\
    va_end(va);					\
  }

#define BUFFER_POPN( B, X )			\
  X B##_popn( TYPE(B) *B, size_t n )		\
  {						\
    X x = B->elements[B->len-1];		\
    resize_##B(B, B->len-n);			\
    return x;					\
  }


#define WRITETO_BUFFER( B, X )				\
  size_t writeto_##B( TYPE(B) *B, X *src, size_t n )	\
  {							\
    size_t old_len = B->len;				\
    resize_##B(B, B->len+n);				\
    X *buf = B->elements+old_len;			\
    memcpy(buf, src, n*sizeof(X));			\
    return B->len;					\
  }

#define READFROM_BUFFER( B, X )				\
  size_t readfrom_##B( X *dst, TYPE(B) *B, size_t n )	\
  {							\
    n = min(n, B->len);					\
    memcpy(dst, B->elements, n * sizeof(X));		\
    return n;						\
  }

#define BUFFER_REF( B, X )			\
  X B##_ref( TYPE(B) *B, long i )		\
  {						\
    if ( i < 0 )				\
      i += B->len;				\
    assert(i > 0 && (size_t)i < B->len);	\
    return B->elements[i];			\
  }

#define BUFFER_SET( B, X )			\
  X B##_set( TYPE(B) *B, long i, X x )		\
  {						\
    if ( i < 0 )				\
      i += B->len;				\
    assert(i > 0 && (size_t)i < B->len);	\
    B->elements[i] = x;				\
    return x;					\
  }

#define BUFFER_SWAP( B, X )			\
  X B##_swap( TYPE(B) *B, long i, X x )		\
  {						\
    if ( i < 0 )				\
      i += B->len;				\
    assert(i > 0 && (size_t)i < B->len);	\
    X out = B->elements[i];			\
    B->elements[i] = x;				\
    return out;					\
  }

#endif
