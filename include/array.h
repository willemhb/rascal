#ifndef rascal_array_h
#define rascal_array_h

#include <string.h>

#include "object.h"
#include "memory.h"

// macros, globals, common array utilities, and array types that don't fit elsewhere
#define resize_vec(p, o, n, t) resize((p), (o)*sizeof(t), (n)*sizeof(t))
#define alloc_vec(n, t)        alloc((n)*sizeof(t))
#define dealloc_vec(p, n, t)   dealloc((p), (n)*sizeof(t))
#define duplicate_vec(p, n, t) duplicate_bytes((p), (n) * sizeof(t))

// array describe macros
#define ARRAY_SLOTS(V)					\
  arity_t   cap;					\
  arity_t   len;					\
  V        *data;					\

#define ARRAY_INIT(T, V, Ctype, type)			\
  void init_##T(T##_t *T)				\
  {							\
    init_obj(&T->obj, type, Ctype);			\
    T->len   = 0;					\
    T->cap   = MinCs[type];				\
    T->data = alloc_vec( T->cap, V );			\
  }

#define ARRAY_FREEZE(T, V)			\
  void freeze_##T( T##_t *T )			\
  {						\
    obj_flags(T) |= obj_fl_frozen;		\
    arity_t o     = T->cap;			\
    T->cap        = T->len;			\
    resize_vec(T->data, o, T->cap, V );		\
  }

#define ARRAY_MARK(T, V)					\
  void mark_##T( object_t *obj )					\
  {								\
    T##_t *T = 	(T##_t*)obj;					\
    _Generic(T->data,						\
	     value_t*:mark_vals((value_t*)T->data, T->len),	\
	     default:mark_objs((object_t**)T->data, T->len));	\
  }

#define ARRAY_FREE(T, V)			\
  void free_##T(object_t *obj)			\
  {						\
    T##_t *T = (T##_t*)obj;			\
    dealloc_vec( T->data, T->cap, V );		\
  }

#define ARRAY_RESIZE(T, V, type, padfn)				\
  void resize_##T( T##_t *T, size_t newl)			\
  {								\
    size_t oldc = T->cap;					\
    size_t newc = padfn(newl, T->len, oldc, MinCs[type]);	\
  								\
    if (T->cap != oldc)						\
	T->data = resize_vec(T->data, oldc, newc, V);		\
    T->len = newl;						\
  }

#define ARRAY_WRITE(T, V)				\
  arity_t T##_write( T##_t *T, V *src, arity_t n)	\
  {							\
    arity_t offset = T->len;				\
    resize_##T( T, T->len + n );			\
    memcpy(T->data+offset, src, n * sizeof(V));		\
    return T->len;					\
  }

#define ARRAY_PUSH(T, V)			\
  arity_t T##_push( T##_t *T, V val)		\
  {						\
    arity_t offset = T->len;			\
    resize_##T( T, T->len+1 );			\
    T->data[offset] = val;			\
    return offset;				\
  }

#define ARRAY_APPEND(T, V)			\
  arity_t T##_append( T##_t *T, arity_t n, ...)	\
  {						\
    arity_t offset = T->len;			\
    if (n==0)					\
      return offset;				\
    resize_##T( T, T->len+n );			\
    va_list va;					\
    va_start(va, n);				\
    for (arity_t i=0;i<n;i++)			\
	T->data[offset+i] = va_arg(va, V);	\
    va_end(va);					\
    return offset;				\
  }

#define ARRAY_POP(T, V)				\
  bool T##_pop(T##_t *T, V *buf)		\
  {						\
    if (T->len == 0)				\
      return false;				\
    if (buf)					\
      *buf = T->data[T->len-1];			\
    resize_##T( T, T->len-1 );			\
    return true;				\
  }

#define ARRAY_CLEAR(T, V, type)				\
  void clear_##T(T##_t *T)				\
  {							\
    resize_##T( T, 0 );					\
    memset( T->data, 0, MinCs[type] * sizeof(V) );	\
  }

// utility array types
// array types

#define FROZEN_FL 0x0000000000000000ul
#define ALIST_FL  0x0000000000000001ul
#define STACK_FL  0x0000000000000002ul

struct vector_t
{
  object_t object;
  arity_t  cap;
  arity_t  length;
  value_t *data;
};

// forward declarations
vector_t  *new_vector(void);
void       init_vector(vector_t *vector);
void       mark_vector(object_t *object);
void       resize_vector(vector_t *vector, arity_t newl);
void       free_vector(object_t *object);
void       vector_push(vector_t *vector, value_t value);


#endif
