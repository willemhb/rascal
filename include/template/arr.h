#ifndef rascal_template_arr_h
#define rascal_template_arr_h

#define EMPTY_HEAD

// templates and generic macros for different array types
#define DECL_ALIST_API(T, V)					\
  T##_t *new_##T(arity_t n, V *ini);				\
  void   init_##T(T##_t *array, arity_t n, V *ini);		\
  void   finalize_##T(T##_t *array);				\
  void   mark_##T( T##_t *array);				\
  size_t pad_##T##_size(size_t oldl, size_t newl, size_t oldc);	\
  void   resize_##T(T##_t *array, arity_t n)

#define ALIST_SLOTS(V)				\
  V       *data;				\
  arity_t  len;					\
  arity_t  cap

#define DECL_ALIST(T)						\
  typedef struct T##_t T##_t

#define ALIST_TYPE(T, H, V)			\
  struct T##_t					\
  {						\
    H						\
    ALIST_SLOTS(V);				\
  }

#define PAD_ALIST_SIZE(T, V)						\
  size_t pad_##T##_size(size_t oldl, size_t newl, size_t oldc)		\
  {									\
    return pad_alist_size(oldl, newl, oldc);				\
  }

#define PAD_STACK_SIZE(T, V)						\
  size_t pad_##T##_size(size_t oldl, size_t newl, size_t oldc)		\
  {									\
    return pad_stack_size(oldl, newl, oldc, T##_min_cap);		\
  }

#define RESIZE_ARRAY(T, V)						\
  void resize_##T(T##_t* array, arity_t newl)				\
  {									\
    size_t oldc = array->cap;						\
    size_t newc = pad_##T##_size( array->len, newl, oldc );		\
    if (oldc != newc)							\
      array->data = resizev( array->data, oldc, newc, V );		\
  }

#define NEW_ARRAY(T, V)					\
  T##_t *new_##T(arity_t n, V *ini)			\
  {							\
    T##_t *out = alloc( sizeof(T##_t) );		\
    init_##T( out, n, ini );				\
    return out;						\
  }

#define INIT_ARRAY(T, V)						\
  void init_##T(T##_t *array, arity_t n, V *ini)			\
  {									\
    arity_t cap = pad_##T##_size( 0, n, 0 );				\
    array->len         = n;						\
    array->cap         = cap;						\
    array->data        = allocv(cap, V);				\
    if (ini)								\
      copyv( array->data, ini, n, V );					\
  }

#define MARK_ARRAY(T)				\
  void mark_##T(T##_t *array)			\
  {						\
    trace(array->data, array->len);		\
  }

#define FINALIZE_ARRAY(T, V)				\
  void finalize_##T(T##_t *array)			\
  {							\
    deallocv( array->data, (arity_t)array->cap, V );	\
  }

#endif
