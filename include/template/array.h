#ifndef rascal_array_h
#define rascal_array_h

#define ARRAYLIST(T, V)				\
  struct T##_t					\
  {						\
    OBJ_HEAD;					\
    V      *vals;				\
    arity_t len;				\
    arity_t cap;				\
  }

#define STRING(T, V)				\
  struct T##_t					\
  {						\
    OBJ_HEAD;					\
    V      *vals;				\
    hash_t  h;					\
  }

#define TUPLE(T, V)				\
  struct T##_t					\
  {						\
    OBJ_HEAD;					\
    obj_t *spec;				\
    V slots[0];					\
  }

#define INIT_ALIST(T, V)						\
  void init_##T(obj_t *obj, arity_t n, void *ini, flags_t fl)		\
  {									\
    (void)fl;								\
    T##_t *array = (T##_t*)obj;						\
    array->cap   = max(min_##T##_arity, ceil_log2(n));			\
    array->len   = n;							\
    array->vals	 = alloc_arr( n, sizeof(V) );				\
    if (ini != NULL)							\
      memcpy( array->vals, ini, n * sizeof(V) );			\
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
  						\
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

#define INIT_STRING(T, V, hash_fn)					\
  void init_##T( obj_t *obj, arity_t n, void *ini, flags_t fl )		\
  {									\
    T##_t *string = (T##_t*)obj;					\
    string->arity = n;							\
    string->vals  = ini;						\
    string->hash = hash_fn( ini, n );					\
    if (!is_flag(fl, DATA_STATIC))					\
      string->vals = duplicate( string->vals, n+1 * sizeof(V));		\
  }

#endif
