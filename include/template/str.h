#ifndef rascal_template_str_h
#define rascal_template_str_h

#define DECL_STRING(T, V)			\
  typedef struct T##_ini_t			\
  {						\
    init_t  init;				\
    bool    has_hash;				\
    hash_t  hash;				\
  } T##_ini_t;					\
  struct T##_t					\
  {						\
    OBJ_HEAD;					\
    V      *vals;				\
    arity_t len;				\
    Ctype_t Ctype;				\
    hash_t  hash;				\
  };						\

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
