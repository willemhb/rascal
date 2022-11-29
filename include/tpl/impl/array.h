#ifndef rl_tpl_impl_array_h
#define rl_tpl_impl_array_h

#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "tpl/type.h"

#include "vm/memory.h"

#include "util/collection.h"
#include "util/number.h"

#undef ARRAY

#define ARRAY( ARR, X, V, padfn )					\
  TYPE(ARR) make_##ARR( size_t n, X *ini )				\
  {									\
    size_t cap = padfn(n, 0, 0);					\
    HEADER(ARR) *out = alloc_table(sizeof(HEADER(ARR)), cap, sizeof(X)); \
    out->len = n;							\
    out->cap = cap;							\
    if ( ini )								\
      memcpy(out->elements, ini, n * sizeof(X));			\
    return out->elements;						\
  }									\
  									\
  void resize_##ARR( TYPE(ARR) *ARR, size_t new_len )			\
  {									\
    size_t old_cap = ARR##_cap(*ARR);					\
    size_t old_len = ARR##_len(*ARR);					\
    size_t new_cap = padfn(new_len, old_len, old_cap);			\
    if ( new_cap != old_cap )						\
      {									\
	HEADER(ARR) *new_space  = adjust_table(ARR##_header(*ARR),	\
					       sizeof(HEADER(ARR)),	\
					       old_cap,			\
					       new_cap,			\
					       sizeof(X));		\
	new_space->cap = new_cap;					\
	*ARR = new_space->elements;					\
      }									\
    ARR##_header(*ARR)->len = new_len;					\
  }									\
  									\
  void trim_##ARR( TYPE(ARR) *ARR )					\
  {									\
    *ARR = adjust_table(ARR##_header(*ARR),				\
			sizeof(HEADER(ARR)),				\
			ARR##_len(*ARR),				\
			sizeof(X));					\
  }									\
  									\
  TYPE(ARR) dup_##ARR( TYPE(ARR) ARR )					\
  {									\
    return make_##ARR(ARR##_len(ARR), ARR);				\
  }									\
									\
  void free_##ARR( TYPE(ARR) ARR )					\
  {									\
    dealloc_table(ARR##_header(ARR),					\
		  sizeof(HEADER(ARR)),					\
		  ARR##_cap(ARR),					\
		  sizeof(X));						\
  }									\
  									\
  HEADER(ARR) *ARR##_header( TYPE(ARR) ARR )				\
  {									\
    return (HEADER(ARR)*)(((uchar*)ARR)-sizeof(HEADER(ARR)));		\
  }									\
  									\
  size_t ARR##_len( TYPE(ARR) ARR )					\
  {									\
    return ARR##_header(ARR)->len;					\
  }									\
  									\
  size_t ARR##_cap( TYPE(ARR) ARR )					\
  {									\
    return ARR##_header(ARR)->cap;					\
  }									\
  									\
  size_t ARR##_size( TYPE(ARR) ARR )					\
  {									\
    return ARR##_len(ARR) * sizeof(X);					\
  }									\
									\
  size_t ARR##_alloc( TYPE(ARR) ARR )					\
  {									\
    return ARR##_cap(ARR) * sizeof(X);					\
  }									\
  									\
  size_t ARR##_cpy( TYPE(ARR) ARR, X *src, size_t n )			\
  {									\
    size_t cpysize = min(n, ARR##_len(ARR));				\
    memcpy(ARR, src, cpysize * sizeof(X));				\
    return cpysize;							\
  }									\
  									\
  void ARR##_fill( TYPE(ARR) ARR, X x )					\
  {									\
    for ( size_t i=0; i<ARR##_len(ARR); i++ )				\
      ARR[i] = x;							\
  }									\
  									\
  size_t ARR##_push( TYPE(ARR) *ARR, X x )				\
  {									\
    size_t loc = ARR##_len(*ARR);					\
    resize_##ARR(ARR, loc+1);						\
    (*ARR)[loc] = x;							\
    return loc+1;							\
  }									\
  									\
  X ARR##_pop( TYPE(ARR) *ARR )						\
  {									\
    size_t len = ARR##_len(*ARR);					\
    assert(len > 0);							\
    X out = (*ARR)[len-1];						\
    resize_##ARR(ARR, len-1);						\
    return out;								\
  }									\
  									\
  size_t ARR##_pushn( TYPE(ARR) *ARR, size_t n, ... )			\
  {									\
    size_t len = ARR##_len(*ARR);					\
    resize_##ARR(ARR, len+n);						\
    va_list va; va_start(va, n);					\
    for ( size_t i=0; i<n; i++ )					\
      (*ARR)[len+i] = va_arg(va, V);					\
    va_end(va);								\
    return len+n;							\
  }									\
  									\
  X ARR##_popn( TYPE(ARR) *ARR, size_t n )				\
  {									\
    size_t len = ARR##_len(*ARR);					\
    size_t loc = len-n;							\
    X out = (*ARR)[loc];						\
    resize_##ARR(ARR, loc);						\
    return out;								\
  }									\
  									\
  X ARR##_ref( TYPE(ARR) ARR, long i )					\
  {									\
    size_t len = ARR##_len(ARR);					\
    if (i < 0)								\
      i += len;								\
    assert(i >= 0 && (size_t)i < len);					\
    return ARR[i];							\
  }									\
  									\
  X ARR##_set( TYPE(ARR) ARR, long i, X x )				\
  {									\
    size_t len = ARR##_len(ARR);					\
    if (i < 0)								\
      i += len;								\
    assert(i >= 0 && (size_t)i < len);					\
    ARR[i] = x;								\
    return x;								\
  }									\
  									\
  X ARR##_swap( TYPE(ARR) ARR, long i, X x )				\
  {									\
    size_t len = ARR##_len(ARR);					\
    if (i < 0)								\
      i += len;								\
    assert(i >= 0 && (size_t)i < len);					\
    X out = ARR[i];							\
    ARR[i] = x;								\
    return out;								\
  }

#endif
