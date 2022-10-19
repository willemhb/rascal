#ifndef rascal_obj_array_h
#define rascal_obj_array_h

#include "obj/object.h"

// describe macros
#define DARRAY(V)				\
  OBJECT					\
  size_t  length;				\
  size_t  capacity;				\
  V      *data;

#define VARRAY(V)				\
  OBJECT					\
  size_t  length;				\
  V      *data;

#define ARRAY_ACC_SIG( T, V, method, ... )				\
  V T##_##method( object_t *obj, long n __VA_OPT__(,) __VA_ARGS__ )	\

#define ARRAY_ACC_SIG_S( T, V, method, ... )				\
  bool T##_##method##_s( object_t *obj, long n, V *buf __VA_OPT__(,) __VA_ARGS__ )

#define ARRAY_PUSH_SIG( T, V, method, ... )				\
  size_t T##_##method( object_t *obj, V x __VA_OPT__(,) __VA_ARGS__ )

#define ARRAY_PUSH_SIG_S( T, V, method, ... )				\
  bool T##_##method( object_t *obj, V x, size_t *buf __VA_OPT__(,) __VA_ARGS__ )

#define ARRAY_POP_SIG( T, V, method, ... )			\
  V T##_##method( object_t *obj __VA_OPT__(,) __VA_ARGS__ )

#define ARRAY_POP_SIG_S( T, V, method, ... )	\
  bool T##_##method( object_t *obj, V *buf __VA_OPT__(,) __VA_ARGS__ )

#define ARRAY_MEM_SIG( T, method, ... )				\
  void T##_method( object_t *obj __VA_OPT__(,) __VA_ARGS__ )

#define ARRAY_METHOD_START			\
  T##_t *T = (T##_t*)obj;
  

#define ARRAY_ACC_START				\
  ARRAY_METHOD_START				\
  if (n < 0)					\
    n += T->length;

#define ARRAY_ACC_S_START			\
  ARRAY_ACC_START				\
  if ( n < 0 || (size_t)n >= T->length)		\
    return false;

#define DARRAY_INIT( T, V, pad, minc )		\
  ARRAY_MEM_SIG( T, V, init, size_t n )		\
  {						\
    ARRAY_METHOD_START				\
    						\
  }
  

#define ARRAY_RESIZE( T, V, pad, minc )				\
  ARRAY_MEM_SIG( T, V, resize, size_t newl )			\
  {								\
    ARRAY_METHOD_START						\
      size_t oldc = T->capacity, oldl =	T->length;		\
    size_t newc = pad( oldl, newl, oldc, minc );		\
    T->data = resize( T->data, oldc*sizeof(V), newc*sizeof(V));	\
  }

#define ARRAY_REF(T, V)				\
  ARRAY_ACC_SIG( T, V, ref )			\
  {						\
    ARRAY_ACC_START				\
    return T->data[n];				\
  }

#define ARRAY_REF_S(T, V)				\
  ARRAY_ACC_SIG_S( T, V, ref )				\
  {							\
    ARRAY_ACC_S_START				\
      if (buf)						\
	*buf = T->data[n];				\
    return true;					\
  }

#define ARRAY_SET(T, V)					\
  ARRAY_ACC_SIG( T, V, set, V x )				\
  {							\
    ARRAY_ACC_START					\
      V old = T->data[n];				\
    T->data[n] = x;					\
    return old;						\
  }

#define ARRAY_SET_S( T, V )				\
  ARRAY_ACC_SIG_S( T, V, set, V x )			\
  {							\
    ARRAY_ACC_S_START					\
      if ( buf )					\
	{						\
	  *buf = T->data[n];				\
	  T->data[n] = x;				\
	}						\
    return true;					\
  }

#define ARRAY_PUSH( T, V )			\
  size_t					\
  {						\
  						\
  }

#define ARRAY_PUT( T, V )			\
  ARRAY_ACC_SIG( T, V, put, V x )		\
  {						\
    ARRAY_ACC_START				\
      if ( (size_t)n == T->length-1)		\
						\
  }						\

// generic array type
typedef struct
{
  DARRAY(void)
} array_t;

// forward declarations
size_t    pad_alist_size( size_t oldl, size_t newl, size_t oldc, size_t newc );
size_t    pad_stack_size( size_t oldl, size_t newl, size_t oldc, size_t newc );
object_t *new_darray( type_t *type, size_t n );


#endif
