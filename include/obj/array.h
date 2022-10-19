#ifndef rascal_obj_array_h
#define rascal_obj_array_h

#include "obj/object.h"

// describe macros (there's a lot of these but they should cover every use case)
#define ARRAY(V)				\
  OBJECT					\
  size_t  length;				\
  size_t  capacity;				\
  V      *data;

#define ARRAY_ACC_SIG( T, V, method, ... )				\
  V T##_##method( object_t *obj, long n __VA_OPT__(,) __VA_ARGS__ )	\
    
#define ARRAY_ACC_SIG_S( T, V, method, ... )				\
  bool T##_##method##_s( object_t *obj, long n, V *buf __VA_OPT__(,) __VA_ARGS__ )

#define ARRAY_ADD_SIG( T, V, method, ... )				\
  size_t T##_##method( object_t *obj, V x __VA_OPT__(,) __VA_ARGS__ )

#define ARRAY_ADD_SIG_S( T, V, method, ... )				\
  bool T##_##method( object_t *obj, V x, size_t *buf __VA_OPT__(,) __VA_ARGS__ )

#define ARRAY_POP_SIG( T, V, method, ... )			\
  V T##_##method( object_t *obj __VA_OPT__(,) __VA_ARGS__ )

#define ARRAY_POP_SIG_S( T, V, method, ... )	\
  bool T##_##method( object_t *obj, V *buf __VA_OPT__(,) __VA_ARGS__ )

#define ARRAY_MEM_SIG( T, method, ... )				\
  void method##_##T( object_t *obj __VA_OPT__(,) __VA_ARGS__ )

#define ARRAY_METHOD_START			\
  T##_t *T = (T##_t*)obj

#define ARRAY_ACC_START				\
  ARRAY_METHOD_START;				\
  if (n < 0)					\
    n += T->length

#define ARRAY_ACC_S_START			\
  ARRAY_ACC_START;				\
  if ( n < 0 || (size_t)n >= T->length)		\
    return false

#define FARRAY_INIT( T, V, pad )					\
  ARRAY_MEM_SIG( T, V, init, type_t *type, size_t n, void * data )	\
  {									\
    ARRAY_METHOD_START;							\
    T->length  = n;							\
    T->capacity= pad( 0, n, 0, 0 );					\
    if ( data )								\
      memcpy( T->data, data, n * sizeof(V) );				\
  }

#define ARRAY_INIT( T, V, pad, minc )					\
  ARRAY_MEM_SIG( T, V, init, type_t *type, size_t n, void * data )	\
  {									\
    ARRAY_METHOD_START;							\
    size_t oldc = 0, oldl = 0;						\
    size_t newc = pad( oldl, n, oldc, minc );				\
    T->length   = n;							\
    T->capacity = newc;							\
    T->data     = alloc( newc * sizeof( V ) );				\
    if ( data )								\
      memcpy( T->data, data, n * sizeof( V ) );				\
  }

#define ARRAY_RESIZE( T, V, pad, minc )					\
  ARRAY_MEM_SIG( T, V, resize, size_t newl )				\
  {									\
    ARRAY_METHOD_START;							\
    size_t oldc = T->capacity, oldl =	T->length;			\
    size_t newc = pad( oldl, newl, oldc, minc );			\
    T->length   = newl;							\
    if (newc == oldc)							\
      return;								\
    T->data = resize( T->data, oldc*sizeof(V), newc*sizeof(V));		\
  }

#define ARRAY_REF( T, V )			\
  ARRAY_ACC_SIG( T, V, ref )			\
  {						\
    ARRAY_ACC_START;				\
    return T->data[n];				\
  }

#define ARRAY_REF_S( T, V )				\
  ARRAY_ACC_SIG_S( T, V, ref )				\
  {							\
    ARRAY_ACC_S_START;					\
    if (buf)						\
      *buf = T->data[n];				\
    return true;					\
  }

#define ARRAY_SET( T, V )					\
  ARRAY_ACC_SIG( T, V, set, V x )				\
  {								\
    ARRAY_ACC_START;						\
    V old = T->data[n];						\
    T->data[n] = x;						\
    return old;							\
  }

#define ARRAY_SET_S( T, V )				\
  ARRAY_ACC_SIG_S( T, V, set, V x )			\
  {							\
    ARRAY_ACC_S_START;					\
    if ( buf )						\
      {							\
	*buf = T->data[n];				\
	T->data[n] = x;					\
      }							\
    return true;					\
  }

#define ARRAY_PUSH( T, V )			\
  ARRAY_ADD_SIG( T, V, push )			\
  {						\
    ARRAY_METHOD_START;				\
    resize_##T( T, T->length+1 );		\
    T->data[T->length-1] = x;			\
    return T->length-1;				\
  }

#define ARRAY_INSERT( T, V )						\
  ARRAY_ADD_SIG( T, V, insert, V x, long n )				\
  {									\
    ARRAY_METHOD_START;							\
    if( n == -1 )							\
      return T##_push( obj, x );					\
    if ( n < 0 )							\
      n += T->length;							\
    resize_##T( T, T->length + 1 );					\
    memmove( T->data+n+1, T->data+n, (T->length-n-1)*sizeof( V ) );	\
    T->data[n] = x;							\
    return n;								\
  }

#define ARRAY_INSERT_S( T, V )						\
  ARRAY_ADD_SIG_S( T, V, insert, V x, long n )				\
  {									\
    ARRAY_METHOD_START;							\
    if ( n == -1 )							\
      {									\
	size_t off = T##_push( obj, x );				\
	if (buf)							\
	  *buf = off;							\
	return true;							\
      }									\
    if ( n < 0 )							\
      n += T->length;							\
    if ( n < 0 || n >= T-length )					\
      return false;							\
    resize_##T( T, T->length + 1 );					\
    memmove( T->data+n+1, T->data+n, ( T->length-n-1 )*sizeof( V ) );	\
    T->data[n] = x;							\
    if ( buf )								\
      *buf = n;								\
    return true;							\
  }

#define ARRAY_POP( T, V )			\
  ARRAY_POP_SIG( T, V, pop )			\
  {						\
    ARRAY_METHOD_START;				\
    V out = T->data[T->length-1];		\
    resize_##T( obj, T->length-1);		\
    return out;					\
  }

#define ARRAY_POP_S( T, V )			\
  ARRAY_POP_SIG_S( T, V, pop )			\
  {						\
    ARRAY_METHOD_START;				\
    if ( T->length == 0 )			\
      return false;				\
    if ( buf )					\
      *buf = T->data[T->length-1];		\
    resize_##T( obj, T->length-1);		\
    return true;				\
  }

#define ARRAY_POPAT( T, V )						\
  ARRAY_POP_SIG( T, V, popat, long n )					\
  {									\
    ARRAY_METHOD_START;							\
    if ( n == -1 )							\
      return T##_pop( obj );						\
    if ( n < 0 )							\
      n += T->length;							\
    V out = T->data[n];							\
    memmove( T->data+n, T->data+n+1, ( T->length-n-1 )*sizeof( V ) );	\
    resize_##T( obj, T->length-1 );					\
    return out;								\
  }

#define ARRAY_POPAT_S( T, V )						\
  ARRAY_POP_SIG_S( T, V, popat, long n )				\
  {									\
    ARRAY_METHOD_START;							\
    if ( n == -1 )							\
      return T##_pop_s( obj, buf );					\
    if ( n < 0 )							\
      n += T->length;							\
    if ( n < 0 || (size_t)n > T->length )				\
      return false;							\
    if( buf )								\
      *buf = T->data[n];						\
    memmove( T->data+n, T->data+n+1, ( T->length-n-1 )*sizeof( V ) );	\
    resize_##T( obj, T->length-1 );					\
    return true;							\
  }

// generic array type
typedef struct
{
  ARRAY(void)
} array_t;

// forward declarations
size_t    pad_alist_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t    pad_stack_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t    pad_string_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t    pad_buffer_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
object_t *new_darray( type_t *type, size_t n ); // alist
object_t *new_sarray( type_t *type, size_t n ); // stack
object_t *new_barray( type_t *type, size_t n ); // buffer
object_t *new_farray( type_t *type, size_t n ); // fixed
object_t *new_tarray( type_t *type, size_t n ); // string

#endif
