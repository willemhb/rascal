#ifndef rascal_table_h
#define rascal_table_h

#define TABLE(T, K, V)				\
  typedef struct T##_kv_t			\
  {						\
    OBJ_HEAD;					\
    K      key;					\
    V      val;					\
    hash_t hash;				\
  } T##_kv_t;					\
						\
  struct T##_t					\
  {						\
    OBJ_HEAD;					\
    arity_t len;				\
    arity_t cap;				\
    T##_kv_t **entries;				\
  }

#endif
