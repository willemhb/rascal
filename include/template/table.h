#ifndef rascal_table_h
#define rascal_table_h


#define DECL_TABLE(T)				\
  typedef struct T##_t	T##_t

#define TABLE_SLOTS(H, E)			\
  H						\
  E *data;					\
  arity_t len;					\
  arity_t cap

#define TABLE_TYPE(T, H, E)			\
  struct T##_t					\
  {						\
    TABLE_SLOTS(H, E);				\
  }

#define DECL_ORDERED_TABLE(T, E)				\
								\
  typedef struct T##_map_t					\
  {								\
    int     *data;						\
    arity_t  len;						\
    arity_t  cap;						\
  } T##_map_t;							\
  typedef struct T##_t						\
  {								\
    E        *data;						\
    arity_t   len;						\
    arity_t   cap;						\
    T##_map_t map;						\
  } T##_t

#define DECL_TABLE_API(T, K, E)						\
  size_t pad_##T##_size(size_t oldl, size_t newl, size_t oldc);		\
  void   T##_init(T##_t *table, arity_t n, E* ini);			\
  bool   T##_get(T##_t *table, K key, E *buf );				\
  bool 	 T##_put(T##_t *table, K key, E *buf );				\
  bool	 T##_pop(T##_t *table, K key, E *buf );				\
  void   rehash_##T(E *old, E *new, size_t oldn,size_t newn)

#define TABLE_TRACE(T, O)			\
  void trace_##T(T##_t *table)			\
  {						\
    trace( (O*)table->data, table->cap );	\
  }

#define TABLE_INIT(T, E)						\
    void   T##_init(T##_t *table, arity_t n, E* ini)			\
    {									\
      arity_t cap = pad_##T##_size( 0,					\
				    n,					\
				    T##_min_cap );			\
      table->len     = n;						\
      table->cap     = cap;						\
      table->entries = allocv( cap, E );				\
      if (ini)								\
	copyv( table->entries, ini, cap, E* );				\
  }

#define TABLE_PAD(T)							\
  size_t pad_##T##_size(size_t oldl, size_t newl, size_t oldc)		\
  {									\
    size_t newc = oldc;							\
    if (oldl > newl)							\
      while (newc > T##_min_cap						\
	     && newl < newc / 2 * T##_load)				\
	  new_cap >>= 1;						\
    else if (newl > oldl)						\
      while (newl > newc *T##_load )					\
	newc <<= 1;							\
    return newc;							\
  }

#define TABLE_FINALIZE(T, E, O)			\
  void finalize_##T(T##_t *table)		\
  {						\
    deallocv( (O*)table->entries,		\
	      table->cap,			\
	      E );				\
  }

#define TABLE_REHASH(T, E, guard, hash)					\
  void rehash_##T(E *old,E *new,arity_t oldc, arity_t newc)		\
  {									\
    for (arity_t i=0; i<= oldc; i++)					\
      {									\
	E entry = old[i];						\
	if (guard(entry))						\
	  continue;							\
	hash_t h = hash(entry);						\
	while (!guard(entry))						\
	  {								\
	    i++;							\
	    if (i >= newc)						\
	      i = 0;							\
	  }								\
	new[i] = entry;							\
      }									\
  }

#define TABLE_RESIZE(T, E)						\
  void resize_##T(T##_t *table, size_t newl)				\
  {									\
    table->len  = newl;							\
    size_t oldc = table->cap;						\
    size_t newc = pad_table_size(newl,					\
				 T##_load,				\
				 old_cap,				\
				 T##_min_cap );				\
    if (oldc != newc)							\
      {									\
	E *new = allocv(newc, E );					\
	rehash_##T(table->data, new, oldc, newc );			\
	deallocv( table->data, oldc, E );				\
	table->data = new;						\
      }									\
  }


#define TABLE_GET(T, E, K, guard, ehash, cmp, hash)		\
  bool_t T##_get(T##_t *table, K k, E *buf)			\
    {								\
      E *entries   = table->data;				\
      E  entry;						        \
      hash_t h     = hash(k);					\
      arity_t idx  = h  & (table->cap-1);			\
      								\
      while (!guard((entry=entries[idx])))			\
	{							\
	  if (h == ehash(entry) && cmp(k, entry) == 0)		\
	    {							\
	      if (buf)						\
		*buf = entry;					\
	      return true;					\
	    }							\
	  idx++;						\
	  if (idx >= table->cap)				\
	    idx = 0;						\
	}							\
      return false;						\
    }

#define TABLE_PUT(T, E, K, guard, cmp, hash, intern)			\
  bool_t T##_put(T##_t *table, K k, E *buf)				\
  {									\
    E *entries = table->data;						\
    E  entry;								\
    hash_t h = hash(k);							\
    arity_t idx  = h  & (table->cap-1);					\
									\
    while (!guard((entry=entries[idx])))				\
      {									\
	if (h == ehash(entry) && cmp(k, entry) == 0)			\
	  {								\
	      if (buf)							\
		*buf = entry;						\
	      return false;						\
	  }								\
	idx++;								\
	if (idx >= table->cap)						\
	  idx = 0;							\
      }									\
    									\
    entry = intern( table, k, hash, idx );				\
    entries[idx] = entry;						\
    resize_##T( table, table->len+1 );					\
    if (buf)								\
      *buf = (obj_t*)entry;						\
    return true;							\
    }

#define TABLE_POP(T, E, K, N, guard, cmp, hash)				\
  bool_t T##_pop(T##_t *table, K k, E *buf)				\
  {									\
    E *entries = table->data;						\
    E  entry;								\
    hash_t h = hash(k);							\
    arity_t idx  = h  & (table->cap-1);					\
									\
    while (!guard((entry=entries[idx])))				\
      {									\
	if (ehash(entry) == h && cmp(k, entry) == 0)			\
	  {								\
	      if (buf)							\
		*buf = entry;						\
	      entries[idx] = N;						\
	      resize_##T( table, table->len-1);				\
	      return true;						\
	  }								\
	idx++;								\
	  if (idx >= table->cap)					\
	    idx = 0;							\
      }									\
    return false;							\
    }


#endif
