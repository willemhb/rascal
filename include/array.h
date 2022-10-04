#ifndef rascal_array_h
#define rascal_array_h

#include <string.h>

#include "obj.h"
#include "memory.h"

// macros, globals, common array utilities, and array types that don't fit elsewhere
#define resize_vec(p, o, n, t) resize((p), (o)*sizeof(t), (n)*sizeof(t))
#define alloc_vec(n, t)   alloc((n)*sizeof(t))
#define dealloc_vec(p, n, t) dealloc((p), (n)*sizeof(t))

// array describe macros
#define ARRAY_SLOTS(V)					\
  V        *data;					\
  arity_t   len;					\
  arity_t   cap

#define ARRAY_INIT(T, V, Ctype, type)			\
  void init_##T(T##_t *T)				\
  {							\
    init_obj(&T->obj, type, Ctype);			\
    T->len   = 0;					\
    T->cap   = MinCs[type];				\
    T->data = alloc_vec( T->cap, V );			\
  }

#define ARRAY_TRACE(T, V)					\
  void trace_##T( obj_t *obj )					\
  {								\
    T##_t *T = 	(T##_t*)obj;					\
    _Generic(T->data,						\
	     val_t*:trace_vals((val_t*)T->data, T->len),	\
	     default:trace_objs((obj_t**)T->data, T->len));	\
  }

#define ARRAY_FREE(T, V)			\
  void free_##T(obj_t *obj)			\
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

// globals
extern const size_t MinCs[N_TYPES];

// utility array types
// array types
struct alist_t
{
  obj_t obj;
  ARRAY_SLOTS(val_t);
};

struct stack_t
{
  obj_t obj;
  ARRAY_SLOTS(val_t);
};

// forward declarations
alist_t  *new_alist(void);
void      init_alist(alist_t *alist);
void      trace_alist(obj_t *obj);
void      free_alist(obj_t *obj);
void      resize_alist(alist_t *alist, size_t newl);
arity_t   alist_push(alist_t *alist, val_t val);
arity_t   alist_write(alist_t *alist, val_t *src, arity_t n);
bool      alist_pop(alist_t *alist, val_t *buf);

stack_t  *new_stack(void);
void      trace_stack(obj_t *obj);
void      init_stack(stack_t *stack);
void      free_stack(obj_t *obj);
void      resize_stack(stack_t *stack, size_t newl);
arity_t   stack_push(stack_t *stack, val_t val);
arity_t   stack_write(stack_t *stack, val_t *src, arity_t n);
bool      stack_pop(stack_t *stack, val_t *buf);

size_t pad_alist_size(size_t oldl, size_t newl, size_t oldc, size_t minc);
size_t pad_stack_size(size_t oldl, size_t newl, size_t oldc, size_t minc);

// convenience
#define as_stack(val)    ((stack_t*)as_ptr(val))


#endif
