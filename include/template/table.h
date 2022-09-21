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
    arity_t   len;				\
    arity_t   cap;				\
    obj_t   **entries;				\
  }

#define TABLE_GET(T, K, V, hash_fn, compare_fn)			\
    bool_t T##_get(obj_t *obj, K k, obj_t **buf)		\
    {								\
      T##_t     *table   = (T##_t*)obj;				\
      T##_kv_t **entries = (T##_kv_t**)table->entries;		\
      T##_kv_t  *entry   = NULL;				\
      hash_t h     = hash_fn(k);				\
      arity_t idx  = h  & (table->capacity-1);			\
      								\
      while ((entry=entries[idx]))				\
	{							\
	  if (entry->hash == h && compare_fn(k, entry->key))	\
	    {							\
	      if (buf)						\
		*buf = (obj_t*)entry;				\
	      return true;					\
	    }							\
	  idx++;						\
	  if (idx >= table->capacity)				\
	    idx = 0;						\
	}							\
      return false;						\
    }								

#define TABLE_RESIZE(T, K, V)			\
  obj_t *

#endif
