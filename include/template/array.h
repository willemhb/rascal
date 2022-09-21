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

#define INIT_ALIST(T, V)						\
  void init_##T( obj_t *obj, arity_t n_args, void *ini, flags_t fl )	\
  {									\
    T##_t *array = (T##_t*)obj;						\
    									\
    									\
  }									\

#endif
