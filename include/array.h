#ifndef rascal_array_h
#define rascal_array_h

#include <string.h>

#include "object.h"
#include "memory.h"
#include "utils.h"

// macros, globals, common array utilities, and array types that don't fit elsewhere
#define resize_vec(p, o, n, t) resize((p), (o)*sizeof(t), (n)*sizeof(t))
#define alloc_vec(n, t)        alloc((n)*sizeof(t))
#define dealloc_vec(p, n, t)   dealloc((p), (n)*sizeof(t))
#define duplicate_vec(p, n, t) duplicate_bytes((p), (n) * sizeof(t))

// array describe macros
#define ARRAY_SLOTS(V)							\
  arity_t  cap;								\
  arity_t  len;								\
  V       *data

#define ARRAY_INIT(T, V, minc)					\
  void init_##T(T##_t *T)					\
  {								\
    T->length     = 0;						\
    T->capacity   = minc;					\
    T->data       = alloc_vec( T->capacity, V );		\
  }

#define ARRAY_FREEZE(T, V)			\
  void freeze_##T( T##_t *T )			\
  {						\
    obj_flags(T) |= obj_fl_frozen;		\
    size_t o     = T->capacity;			\
    T->capacity        = T->length;			\
    resize_vec(T->data, o, T->capacity, V );		\
  }

#define ARRAY_MARK(T, V)						\
  void mark_##T( T##_t *T )						\
  {									\
    _Generic(T->data,							\
	     value_t*:mark_values((value_t*)T->data, T->length),	\
	     default:mark_objects((object_t**)T->data, T->length));	\
  }

#define ARRAY_FREE(T, V)				\
  void free_##T(T##_t *T)				\
  {							\
    dealloc_vec( T->data, T->capacity, V );		\
  }

#define ARRAY_RESIZE(T, V, padfn, minc)				\
  void resize_##T( T##_t *T, size_t newl)			\
  {								\
    size_t oldc = T->capacity;					\
    size_t newc = padfn(newl, T->length, oldc, minc);		\
    								\
    if (T->capacity != oldc)					\
      T->data = resize_vec(T->data, oldc, newc, V);		\
    T->length = newl;						\
  }

#define ARRAY_WRITE(T, V)				\
  size_t T##_write( T##_t *T, V *src, size_t n)	\
  {							\
    size_t offset = T->length;				\
    resize_##T( T, T->length + n );			\
    memcpy(T->data+offset, src, n * sizeof(V));		\
    return T->length;					\
  }

#define ARRAY_PUSH(T, V)			\
  size_t T##_push( T##_t *T, V val)		\
  {						\
    size_t offset = T->length;			\
    resize_##T( T, T->length+1 );			\
    T->data[offset] = val;			\
    return offset;				\
  }

#define ARRAY_APPEND(T, V)			\
  size_t T##_append( T##_t *T, size_t n, ...)	\
  {						\
    size_t offset = T->length;			\
    if (n==0)					\
      return offset;				\
    resize_##T( T, T->length+n );			\
    va_list va;					\
    va_start(va, n);				\
    for (size_t i=0;i<n;i++)			\
	T->data[offset+i] = va_arg(va, V);	\
    va_end(va);					\
    return offset;				\
  }

#define ARRAY_POP(T, V)				\
  bool T##_pop(T##_t *T, V *buf)		\
  {						\
    if (T->length == 0)				\
      return false;				\
    if (buf)					\
      *buf = T->data[T->length-1];			\
    resize_##T( T, T->length-1 );			\
    return true;				\
  }

#define ARRAY_CLEAR(T, V, minc)				\
  void clear_##T(T##_t *T)				\
  {							\
    resize_##T( T, 0 );					\
    memset( T->data, 0, minc * sizeof(V) );		\
  }

// utility array types
// array types

typedef union
{
  char      *c8;
  char16_t  *c16;
  char32_t  *c32;

  byte      *u8;
  uint16_t  *u16;

  int8_t    *i8;
  int16_t   *i16;
  int32_t   *i32;

  object_t **objects;
  value_t   *values;
} elements_t;

struct alist_t
{
  object_t   object;

  arity_t    len;
  arity_t    cap;
  elements_t elements;
  byte       isStack;
  byte       isAlist;
  arity_t    minCap;
};

struct binary_t
{
  object_t   object;
  arity_t    len;
  Ctype_t    Ctype;
  elements_t elements;
};

struct buffer_t
{
  object_t   object;
  arity_t    len;
  arity_t    cap;
  elements_t elements;
  byte       isStack;
  byte       isAlist;
  byte       Ctype;
  arity_t    minCap;
};

// forward declarations
alist_t  *new_alist( void );
void      init_alist( alist_t *alist, bool isStatic, arity_t minCap, bool isStack, bool isAlist );
void      mark_alist( object_t *object );
hash_t    hash_alist( object_t *object );
ord_t     order_alists( object_t *x, object_t *y );

binary_t *new_binary( void );
void      init_binary( binary_t *binary, bool isStatic, arity_t len, Ctype_t Ctype, void *data );
void      free_binary( object_t *object );
hash_t    hash_binary( object_t *object );
ord_t     order_binaries( object_t *x, object_t *y );

buffer_t *new_buffer( void );
void      init_buffer( buffer_t *buffer, bool isStatic, arity_t minCap, bool isStack, bool isAlist, Ctype_t Ctype );
void      free_buffer( object_t *object );
hash_t    hash_buffer( object_t *object );
ord_t     order_buffers( object_t *x, object_t *y );

#endif
