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
#define ARRAY_SLOTS(V)						\
  size_t   length;						\
  size_t   capacity;						\
  V       *data;						\
  
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
#define ALIST_MINC  1
#define STACK_MINC  128
#define BUFFER_MINC 512

struct vector_t
{
  object_t    object;
  val_alist_t val_alist;
};

typedef struct stack_t
{
  object_t    object;
  val_stack_t val_stack;
} stack_t;

// forward declarations
size_t     pad_alist_size(size_t newl, size_t oldl, size_t oldc, size_t minc);
size_t     pad_stack_size(size_t newl, size_t oldl, size_t oldc, size_t minc);


vector_t  *new_vector(void);
void       init_vector(vector_t *vector);
void       mark_vector(object_t *object);
void       resize_vector(vector_t *vector, size_t newl);
void       free_vector(object_t *object);
void       vector_push(vector_t *vector, value_t value);


#endif
