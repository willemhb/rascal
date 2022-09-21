#ifndef rascal_hamt_h
#define rascal_hamt_h

#define HAMT_TABLE(T, K, V)				\
  struct T##_kv_t					\
  {							\
    OBJ_HEAD;						\
    T##_kv_t *next;					\
    K         key;					\
    V         val;					\
    hash_t    hash;					\
  };							\
  struct T##_t						\
  {							\
    OBJ_HEAD;						\
    arity_t height;					\
    arity_t length;					\
    obj_t  *cache;					\
    obj_t **entries;					\
    size_t  bitmap;					\
  }

#define HAMT_ARRAY(T, V)			\
    struct T##_t				\
    {						\
      OBJ_HEAD;					\
      arity_t height;				\
      arity_t length;				\
      obj_t  *cache;				\
      union {					\
	obj_t **children;			\
	V      *values;				\
      };					\
    }

#endif
