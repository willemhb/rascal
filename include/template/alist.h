#ifndef rascal_template_alist_h
#define rascal_template_alist_h

#define DECL_ALIST(T, V)						\
  struct T##_t								\
  {									\
    OBJ_HEAD;								\
    V      *vals;							\
    arity_t len;							\
    arity_t cap;							\
  };									\
  bool    T##_get( obj_t *obj, arity_t n, V *buf );			\
  bool    T##_set( obj_t *obj, arity_t n, V val, V *buf );		\
  bool	  T##_pop( obj_t *obj, V *buf );				\
  bool    T##_splice( obj_t *obj, arity_t n, V val, V *buf );		\
  bool    T##_unsplice( obj_t *obj, arity_t n, V *buf );		\
  obj_t  *T##_apend( obj_t *obj, V val );				\
  obj_t  *T##_write( obj_t *obj, V *src, arity_t buf_n );		\
  arity_t T##_read( V *buf, obj_t *obj, arity_t n, arity_t cap )

#define INIT_ALIST(T, V)						\
  void init_##T(obj_t *obj, init_t *args)				\
  {									\
    T##_t      *array  = (T##_t*)obj;					\
    T##_init_t *arrini = (T##_init_t*)init;				\
    array->len         = 0;						\
    array->cap         = 0;						\
    array->vals        = NULL;						\
    obj                = resize_##T( obj, 0, args->len );		\
    if ()								\
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
