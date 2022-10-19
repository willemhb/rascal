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
  V T##_##method( object_t *obj, long i __VA_OPT__(,) __VA_ARGS__ )	\
    
#define ARRAY_ACC_SIG_S( T, V, method, ... )				\
  bool T##_##method##_s( object_t *obj, long i, V *buf __VA_OPT__(,) __VA_ARGS__ )

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
  if (i < 0)					\
    i += T->length

#define ARRAY_ACC_S_START			\
  ARRAY_ACC_START;				\
  if ( i < 0 || (size_t)i >= T->length)		\
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

#define ARRAY_TRACE( T, V )				\
  ARRAY_MEM_SIG( T, V, trace )				\
  {							\
    ARRAY_METHOD_START;					\
    _Generic(((V)0),					\
	     value_t:trace_vals,			\
             object_t*:trace_objs)(T->data, T->length);	\
  }

#define ARRAY_REF( T, V )			\
  ARRAY_ACC_SIG( T, V, ref )			\
  {						\
    ARRAY_ACC_START;				\
    return T->data[i];				\
  }

#define ARRAY_REF_S( T, V )				\
  ARRAY_ACC_SIG_S( T, V, ref )				\
  {							\
    ARRAY_ACC_S_START;					\
    if (buf)						\
      *buf = T->data[i];				\
    return true;					\
  }

#define ARRAY_SET( T, V )					\
  ARRAY_ACC_SIG( T, V, set, V x )				\
  {								\
    ARRAY_ACC_START;						\
    V old = T->data[i];						\
    T->data[i] = x;						\
    return old;							\
  }

#define ARRAY_SET_S( T, V )				\
  ARRAY_ACC_SIG_S( T, V, set, V x )			\
  {							\
    ARRAY_ACC_S_START;					\
    if ( buf )						\
      {							\
	*buf = T->data[i];				\
	T->data[i] = x;					\
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

#define ARRAY_PUSHN( T, V )			\
  ARRAY_ADD_SIG( T, V, pushn, size_t n )	\
  {						\
    ARRAY_METHOD_START;				\
    resize_##T( T, T->length+1 );		\
    						\
  }

#define ARRAY_INSERT( T, V )						\
  ARRAY_ADD_SIG( T, V, insert, V x, long i )				\
  {									\
    ARRAY_METHOD_START;							\
    if( i == -1 )							\
      return T##_push( obj, x );					\
    if ( i < 0 )							\
      i += T->length;							\
    resize_##T( T, T->length + 1 );					\
    memmove( T->data+i+1, T->data+i, (T->length-i-1)*sizeof( V ) );	\
    T->data[i] = x;							\
    return i;								\
  }

#define ARRAY_INSERT_S( T, V )						\
  ARRAY_ADD_SIG_S( T, V, insert, V x, long i )				\
  {									\
    ARRAY_METHOD_START;							\
    if ( i == -1 )							\
      {									\
	size_t off = T##_push( obj, x );				\
	if (buf)							\
	  *buf = off;							\
	return true;							\
      }									\
    if ( i < 0 )							\
      i += T->length;							\
    if ( i < 0 || i >= T-length )					\
      return false;							\
    resize_##T( T, T->length + 1 );					\
    memmove( T->data+i+1, T->data+i, ( T->length-i-1 )*sizeof( V ) );	\
    T->data[i] = x;							\
    if ( buf )								\
      *buf = i;								\
    return true;							\
  }

#define ARRAY_INSERTN( T, V )						\
  ARRAY_ADD_SIG( T, V, insertn, V *src, long i, size_t n )		\
  {									\
    ARRAY_METHOD_START;							\
    if( i == -1 )							\
      return T##_pushn( obj, x, n );					\
    if ( i < 0 )							\
      i += T->length;							\
    resize_##T( T, T->length + n );					\
    memmove( T->data+i+n, T->data+i, (T->length-i-n)*sizeof( V ) );	\
    memcpy( T->data+i, src, n * sizeof( V ) );				\
    return i;								\
  }

#define ARRAY_INSERTN_S( T, V )						\
  ARRAY_ADD_SIG_S( T, V, insertn, V *src, long i, size_t n )		\
  {									\
    ARRAY_METHOD_START;							\
    if( i == -1 )							\
      return T##_pushn_s( obj, x, n );					\
    if ( i < 0 )							\
      i += T->length;							\
    if ( i < 0 || i >= T-length )					\
      return false;							\
    resize_##T( T, T->length + n );					\
    memmove( T->data+i+n, T->data+i, (T->length-i-n)*sizeof( V ) );	\
    memcpy( T->data+i, src, n * sizeof( V ) );				\
    if ( buf )								\
      *buf = i;								\
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


#define ARRAY_POPN( T, V )			\
  ARRAY_POP_SIG( T, V, popn, size_t n )		\
  {						\
    ARRAY_METHOD_START;				\
    V out = T->data[T->length-1];		\
    resize_##T( T, T->length-n);		\
    return out;					\
  }

#define ARRAY_POPN_S( T, V )			\
  ARRAY_POP_SIG_S( T, V, popn, size_t n )	\
  {						\
    ARRAY_METHOD_START;				\
    if ( n > T->length )			\
      return false;				\
    if ( buf )					\
      *buf = T->length+1;			\
    resize_##T( T, T->length-n );		\
    return true;				\
  }

#define ARRAY_POPAT( T, V )						\
  ARRAY_POP_SIG( T, V, popat, long i )					\
  {									\
    ARRAY_METHOD_START;							\
    if ( i == -1 )							\
      return T##_pop( obj );						\
    if ( i < 0 )							\
      i += T->length;							\
    V out = T->data[i];							\
    memmove( T->data+i, T->data+i+1, ( T->length-i-1 )*sizeof( V ) );	\
    resize_##T( obj, T->length-1 );					\
    return out;								\
  }

#define ARRAY_POPAT_S( T, V )						\
  ARRAY_POP_SIG_S( T, V, popat, long i )				\
  {									\
    ARRAY_METHOD_START;							\
    if ( i == -1 )							\
      return T##_pop_s( obj, buf );					\
    if ( i < 0 )							\
      i += T->length;							\
    if ( i < 0 || (size_t)i > T->length )				\
      return false;							\
    if( buf )								\
      *buf = T->data[i];						\
    memmove( T->data+i, T->data+i+1, ( T->length-i-1 )*sizeof( V ) );	\
    resize_##T( obj, T->length-1 );					\
    return true;							\
  }

#define ARRAY_POPN_AT( T, V )						\
  ARRAY_POP_SIG( T, V, popnat, long i, size_t n )			\
  {									\
  ARRAY_METHOD_START;							\
    if ( i == -1 )							\
      return T##_popn( obj, n );					\
    if ( i < 0 )							\
      i += T->length;							\
    V out = T->data[i];							\
    memmove( T->data+i, T->data+i+n, ( T->length-i-n )*sizeof( V ) );	\
    resize_##T( obj, T->length-n );					\
    return out;								\
  }

#define ARRAY_POPN_AT_S( T, V )						\
    ARRAY_POP_SIG( T, V, popnat, long i, size_t n )			\
  {									\
  ARRAY_METHOD_START;							\
    if ( i == -1 )							\
      return T##_popn_s( obj, buf, n );					\
    if ( i < 0 )							\
      i += T->length;							\
    if ( i < 0 || (size_t)i > T->length)				\
      return false;							\
    if ( buf )								\
      *buf = T->data[i];						\
    memmove( T->data+i, T->data+i+n, ( T->length-i-n )*sizeof( V ) );	\
    resize_##T( obj, T->length-n );					\
    return true;							\
  }

#define ARRAY_WRITETO( T, V )						\
  size_t writeto_##T( object_t *dst, V *src, size_t n )			\
  {									\
    ARRAY_METHOD_START;							\
    if ( n == 0 )							\
      return T->length;							\
    resize_##T( obj, T->length + n );					\
    memcpy( T->data, src, n * sizeof( V ) );				\
    return T->length;							\
  }

#define ARRAY_APPENDTO( T, V )						\
  size_t appendto_##T( object_t *obj, size_t n, ... )			\
  {									\
    ARRAY_METHOD_START;							\
    va_list va;								\
    va_start(va,n);							\
    V buffer[n];							\
    for (size_t i=0; i<n; i++)						\
      buffer[i] = va_arg(va, V);					\
    va_end(va);								\
    resize_##T( obj, T->length + n );					\
    memcpy( T->data, buffer, n * sizeof( V ) );				\
    return T->length;							\
  }

#define ARRAY_READFROM( T, V )					\
  size_t readfrom_##T( V *dst, object_t *src, size_t n )	\
  {								\
    ARRAY_METHOD_START;						\
    n =	min( n, T->length );					\
    memcpy( dst, T->data, n * sizeof( V ) );			\
    return n;							\
  }

// forward declarations
size_t    pad_stack_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t    pad_alist_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t    pad_buffer_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t    pad_vector_size( size_t oldl, size_t newl, size_t oldc, size_t minc );
size_t    pad_string_size( size_t oldl, size_t newl, size_t oldc, size_t minc );

object_t *new_stack( type_t *type, size_t n );  // stack size policy
object_t *new_alist( type_t *type, size_t n );  // alist size policy
object_t *new_buffer( type_t *type, size_t n ); // buffer size policy
object_t *new_vector( type_t *type, size_t n ); // vector size policy
object_t *new_string( type_t *type, size_t n ); // string size policy

#endif
