#ifndef rascal_table_h
#define rascal_table_h


#define DECL_TABLE(T, K, V)					\
  typedef struct T##_kv_t					\
  {								\
    OBJ_HEAD;							\
    K      key;							\
    V      val;							\
    hash_t hash;						\
  } T##_kv_t;							\
  typedef struct T##_kv_ini_t					\
  {								\
    T##_t *table;						\
    K      key;							\
    hash_t hash;						\
    V      val;							\
  } T##_kv_ini_t;						\
  typedef struct T##_t						\
  {								\
    OBJ_HEAD;							\
    arity_t   len;						\
    arity_t   cap;						\
    obj_t   **entries;						\
  } T##_t

#define DECL_TABLE_API(T, K, V)						\
  size_t pad_##T##_size(size_t old_size, size_t new_size, size_t cap);	\
  bool   T##_get(obj_t *obj, K key, obj_t **buf );			\
  bool 	 T##_put(obj_t *obj, K key, obj_t **buf );			\
  bool	 T##_pop(obj_t *obj, K key, obj_t **buf );			\
  void   rehash_##T(obj_t **old,obj_t **new,size_t old_n,size_t new_n)


#define TABLE_TRACE(T)				\
  void trace_##T(obj_t *obj)			\
  {						\
    T##_t *table = (T##_t*)obj;			\
    trace( table->entries, table->cap );	\
  }

#define TABLE_INIT(T)							\
  void init_##T(obj_t *obj, arity_t n, void *ini, flags_t fl)		\
  {									\
    (void)fl;								\
    T##_t *table = (T##_t*)obj;						\
    arity_t cap = pad_##T##_size( 0,					\
				  n,					\
				  T##_min_cap );			\
    table->len     = n;							\
    table->cap     = cap;						\
    table->entries = allocv( cap, T##_kv_t* );				\
    if (ini)								\
      copyv( table->entries, ini, cap, T##_kv_t* );			\
  }

#define TABLE_PAD(T)							\
  size_t pad_##T##_size(size_t old_size, size_t new_size, size_t cap)	\
  {									\
    size_t new_cap = cap;						\
    if (old_size > new_size)						\
	while (new_cap > T##_min_cap					\
	       && new_size < new_cap / 2 * T##_load)			\
	  new_cap >>= 1;						\
    else if (new_size > old_size)					\
      while (new_size > new_cap *T##_load )				\
	new_cap <<= 1;							\
    return new_cap;							\
  }

#define TABLE_FINALIZE(T)			\
  void finalize_##T(obj_t *obj)			\
  {						\
    T##_t *table = (T##_t*)obj;			\
    deallocv( table->entries,			\
	      table->cap,			\
	      T##_kv_t* );			\
  }

#define TABLE_REHASH(T)							\
  void rehash_##T(obj_t**old,obj_t**new,arity_t old_n,arity_t new_n)	\
  {									\
    T##_kv_t **old_entries = (T##_kv_t**)old,				\
             **new_entries = (T##_kv_t**)new;				\
    									\
    for (arity_t i=0; i<= old_n; i++)					\
      {									\
	T##_kv_t *entry = old_entries[i];				\
	if (entry == NULL)						\
	  continue;							\
	arity_t i = entry->hash & (new_n-1);				\
	while (new_entries[i] != NULL)					\
	  {								\
	    i++;							\
	    if (i >= new_n)						\
	      i = 0;							\
	  }								\
	new_entries[i] = entry;						\
      }									\
  }

#define TABLE_RESIZE(T, K, V)						\
  void resize_##T(obj_t *obj, size_t n)					\
  {									\
    T##_t* table   = (T##_t*)obj;					\
    table->len     = n;							\
    size_t old_cap = table->cap;					\
    size_t new_cap = pad_table_size(n,					\
				    T##_load,				\
				    old_cap,				\
				    T##_min_cap );			\
    if (old_cap != new_cap)						\
      {									\
	obj_t **new = allocv(new_cap, T##_kv_t* );			\
	rehash_##T(table->entries, new, old_cap, new_cap);		\
	deallocv( table->entries, old_cap, T##_kv_t* );			\
	table->entries = new;						\
      }									\
  }





#define TABLE_GET(T, K, V)					\
    bool_t T##_get(obj_t *obj, K k, obj_t **buf)		\
    {								\
      T##_t     *table   = (T##_t*)obj;				\
      T##_kv_t **entries = (T##_kv_t**)table->entries;		\
      T##_kv_t  *entry   = NULL;				\
      hash_t h     = T##_hash(k);				\
      arity_t idx  = h  & (table->cap-1);			\
      								\
      while ((entry=entries[idx]))				\
	{							\
	  if (entry->hash == h && T##_cmp(k, entry->key))	\
	    {							\
	      if (buf)						\
		*buf = (obj_t*)entry;				\
	      return true;					\
	    }							\
	  idx++;						\
	  if (idx >= table->cap)				\
	    idx = 0;						\
	}							\
      return false;						\
    }

#define TABLE_PUT(T, K, V)					\
    bool_t T##_put(obj_t *obj, K k, obj_t **buf)		\
    {								\
      T##_t     *table   = (T##_t*)obj;				\
      T##_kv_t **entries = (T##_kv_t**)table->entries;		\
      T##_kv_t  *entry   = NULL;				\
      hash_t h     = T##_hash(k);				\
      arity_t idx  = h  & (table->cap-1);			\
      								\
      while ((entry=entries[idx]))				\
	{							\
	  if (entry->hash == h && T##_cmp(k, entry->key))	\
	    {							\
	      if (buf)						\
		*buf = (obj_t*)entry;				\
	      return false;					\
	    }							\
	  idx++;						\
	  if (idx >= table->cap)					\
	    idx = 0;							\
	}								\
      T##_kv_ini_t ini = { table, k, h, NULL };				\
      entry = (T##_kv_t*)construct( T##_kv_type,			\
				    1, &ini, INIT_SPECIAL );		\
      entries[idx] = entry;						\
      resize_##T( (obj_t*)table, table->len+1 );			\
      if (buf)								\
	*buf = (obj_t*)entry;					\
      return true;						\
    }

#define TABLE_POP(T, K, V)					\
    bool_t T##_pop(obj_t *obj, K k, obj_t **buf)		\
    {								\
      T##_t     *table   = (T##_t*)obj;				\
      T##_kv_t **entries = (T##_kv_t**)table->entries;		\
      T##_kv_t  *entry   = NULL;				\
      hash_t h     = T##_hash(k);				\
      arity_t idx  = h  & (table->cap-1);			\
      								\
      while ((entry=entries[idx]))				\
	{							\
	  if (entry->hash == h && T##_cmp(k, entry->key))	\
	    {							\
	      if (buf)						\
		*buf = (obj_t*)entry;				\
	      table->entries[idx] = NULL;			\
	      resize_##T( (obj_t*)table, table->len-1);		\
	      return true;					\
	    }							\
	  idx++;						\
	  if (idx >= table->cap)				\
	    idx = 0;						\
	}							\
      return false;						\
    }


#endif
