#ifndef rl_util_tpl_impl_buffer_h
#define rl_util_tpl_impl_buffer_h

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "tpl/type.h"

#include "vm/memory.h"

#include "util/number.h"
#include "util/collection.h"

#undef BUFFER

#define BUFFER( B, X, V, padfn )					\
  TYPE(B) *make_##B( size_t len )					\
  {									\
    size_t cap  = padfn(len, 0, 0);					\
    TYPE(B)* B  = alloc(sizeof(TYPE(B)));				\
    B->len      = len;							\
    B->cap      = cap;							\
    B->elements = alloc_array(cap, sizeof(X));				\
    return B;								\
  }									\
  									\
  void resize_##B( TYPE(B) *B, size_t new_len )				\
  {									\
    size_t new_cap = padfn(new_len, B->len, B->cap);			\
									\
    if ( new_cap != B->cap )						\
      {									\
	B->elements = adjust_array(B->elements,				\
				   B->cap,				\
				   new_cap,				\
				   sizeof(X));				\
	B->cap = new_cap;						\
      }									\
    B->len = new_len;							\
  }									\
  									\
  void reset_##B( TYPE(B) *B )						\
  {									\
    resize_##B(B, 0);							\
  }									\
  									\
  void free_##B( TYPE(B) *B )						\
  {									\
    dealloc_array(B->elements, B->cap, sizeof(X));			\
    dealloc(B, sizeof(TYPE(B)));						\
  }									\
									\
  void B##_push( TYPE(B) *B, X x )					\
  {									\
    resize_##B(B, B->len+1);						\
    B->elements[B->len-1] = x;						\
  }									\
									\
  X B##_pop( TYPE(B) *B )						\
  {									\
    X x = B->elements[B->len-1];					\
    resize_##B(B, B->len-1);						\
    return x;								\
  }									\
									\
  void B##_pushn( TYPE(B) *B, size_t n, ... )				\
  {									\
    size_t old_len = B->len;						\
    resize_##B(B, B->len+n);						\
    X *cell = B->elements+old_len-1;					\
    va_list va; va_start(va, n);					\
    for (size_t i=0; i<n; i++)						\
      *(cell++) = va_arg(va, V);					\
    va_end(va);								\
  }									\
									\
  X B##_popn( TYPE(B) *B, size_t n )					\
  {									\
    X x = B->elements[B->len-1];					\
    resize_##B(B, B->len-n);						\
    return x;								\
  }									\
  									\
  size_t appendto_##B( TYPE(B) *B, X *src, size_t n )			\
  {									\
    size_t old_len = B->len;						\
    resize_##B(B, B->len+n);						\
    X *buf = B->elements+old_len;					\
    memcpy(buf, src, n*sizeof(X));					\
    return B->len;							\
  }									\
  									\
  size_t writeto_##B( TYPE(B) *B, X *src, size_t n )			\
  {									\
    if ( n > B->len )							\
      resize_##B(B, n);							\
    memcpy(B->elements, src, n*sizeof(X));				\
    return n;								\
  }									\
  size_t readfrom_##B( X *dst, TYPE(B) *B, size_t n )			\
  {									\
    n = min(n, B->len);							\
    memcpy(dst, B->elements, n * sizeof(X));				\
    return n;								\
  }									\
  X *B##_at( TYPE(B) *B, long i )					\
  {									\
    if ( i < 0 )							\
      i += B->len;							\
    assert(i >= 0 && (size_t)i < B->len);				\
    return B->elements+i;						\
  }									\
  X B##_ref( TYPE(B) *B, long i )					\
  {									\
    return *B##_at(B, i);						\
  }									\
  X B##_set( TYPE(B) *B, long i, X x )					\
  {									\
    X* spc = B##_at(B, i);						\
    *spc = x;								\
    return x;								\
  }									\
  X B##_swap( TYPE(B) *B, long i, X x )					\
  {									\
    X* spc = B##_at(B, i);						\
    X out = *spc;							\
    *spc = x;								\
    return out;								\
  }

#endif
