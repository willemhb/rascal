#ifndef rl_tpl_impl_array_h
#define rl_tpl_impl_array_h

#include <assert.h>
#include <string.h>
#include <stdarg.h>

#include "tpl/undef/array.h"
#include "tpl/impl/type.h"

#define MAKE_ARR( Type, A, X )			\
  TYPE(A) make_##A( size_t n, X *ini )		\
  {						\
    return (TYPE(A))make_array(&Type, n, ini);	\
  }

#define FREE_ARR( A, X )			\
  void free_##A( TYPE(A) A )			\
  {						\
    free_array((array_t)A);			\
  }

#define RESIZE_ARR( A, X )				\
  size_t resize_##A( TYPE(A) A, size_t new_count )	\
  {							\
    return resize_array((array_t)A, new_count);		\
  }

#define ARRAY_REF( A, X )				\
  X A##_ref( TYPE(A) A, long i )			\
  {							\
    struct array_t *header = array_header((array_t)A);	\
    TYPE(A) A##_data = (TYPE(A))array_data((array_t)A);	\
    if ( i < 0 )					\
      i += header->len;					\
    assert(i >= 0 && (size_t)i < header->len);		\
    return A##_data[i];					\
  }

#define ARRAY_SET( A, X )					\
  X A##_set( TYPE(A) A, long i, X x )				\
  {								\
    struct array_t *header = array_header((array_t)A);		\
    TYPE(A) A##_data = (TYPE(A))array_data((array_t)A);		\
    if ( i < 0 )						\
      i += header->len;						\
    assert(i >= 0 && (size_t)i < header->len);			\
    A##_data[i] = x;						\
    return A##_data[i];						\
  }

#define ARRAY_ADD( A, X, V )						\
  size_t A##_add( TYPE(A) A, size_t n, ... )				\
  {									\
    struct array_t *header = array_header((array_t)A);			\
    bool is_string = header->obj.type->stringp;				\
    resize_array((array_t)A, header->len+n);				\
    X buf[n+is_string];						\
    va_list va; va_start(va, n);					\
    for ( size_t i=0; i<n; i++ )					\
      buf[i] = va_arg(va, V);					\
    va_end(va);								\
    if ( is_string )							\
      buf[n] = (X)0;							\
    memcpy(A##_data(A), buf, (n+is_string)*sizeof(X));		\
    return header->len;							\
  }

#define ARRAY_POP( A, X )				\
  X A##_pop( TYPE(A) A, size_t n )			\
  {							\
    struct array_t* header = array_header((array_t)A);	\
    assert( n <= header->len);				\
    X out = A##_data(A)[header->len-1];			\
    resize_##A(A, header->len-n);			\
    return out;						\
  }

#endif
