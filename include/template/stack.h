#ifndef rascal_template_stack_h
#define rascal_template_stack_h


#define DECL_STACK(T, V)						\
  struct T##_t								\
  {									\
    OBJ_HEAD;								\
    V      *vals;							\
    arity_t len;							\
    arity_t cap;							\
  };									\
  arity_t T##_push( obj_t *obj, V val );				\
  arity_t T##_pushn( obj_t *obj, arity_t n );				\
  V       T##_pop( obj_t *obj );					\
  V       T##_popn( obj_t *obj )

#define INIT_ALIST(T, V)						\
  void init_##T(obj_t *obj, init_t *args)				\
  {									\
    T##_t      *array  = (T##_t*)obj;					\
    T##_init_t *arrini = (T##_init_t*)init;				\
    array->len         = 0;						\
    array->cap         = 0;						\
    array->vals        = NULL;						\
    obj                = resize_##T( obj, 0, args->len );		\
  }

#define TRACE_ALIST(T, V)			\
  void trace_##T(obj_t *obj)			\
  {						\
    T##_t *array = (T##_t*)obj;			\
    trace(array->vals, array->len);		\
  }

#define FINALIZE_ALIST(T, V)			\
  void finalize_##T(obj_t *obj)			\
  {						\
    T##_t *array = (T##_t*)obj;			\
    deallocv( array->vals, V );			\
  }

#define RESIZE_ALIST(T, V)						\
  obj_t *reallocate_##T(obj_t* obj, size_t n)				\
  {									\
    T##_t * array = (T##_t*)obj;					\
    arity_t old_len = array->len, old_cap = array->cap;			\
    array->len = n;							\
    if (n > array->cap)							\
	array->cap <<= 1;						\
    else if (n < array->cap>>1)						\
	array->cap >>= 1;						\
    if (array->cap != old_cap)						\
      array->vals = reallocate( ((void*)array->vals),			\
				array->cap * sizeof(V) );		\
    return obj;								\
  }


#endif
