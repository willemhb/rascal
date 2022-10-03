#ifndef rascal_template_h
#define rascal_template_h

// array describe macros
#define ARRAY_SLOTS(V)					\
  V          *data;					\
  arity32_t   len;					\
  arity32_t   cap

#define OBJ_NEW(T)				\
  T##_t *new_##T(void)				\
  {						\
    return alloc(sizeof(T##_t));		\
  }

#define ARRAY_INIT(T, V, Ctype, type)			\
  void init_##T(T##_t *T)				\
  {							\
    init_obj(&T->obj, type, Ctype);			\
    T->len   = 0;					\
    T->cap   = Mincs[type];				\
    T->data = alloc_vec( T->cap, V );			\
  }

#define ARRAY_TRACE(T, V)					\
  void trace_##T( obj_t *obj )					\
  {								\
    T##_t *T = 	(T##_t*)obj;					\
    _Generic(T->data,						\
	     val_t*:trace_vals((val_t*)T->data, T->len),	\
	     default:trace_objs((obj_t**)T->data, T->len));	\
  }

#define ARRAY_FREE(T, V)			\
  void free_##T(obj_t *obj)			\
  {						\
    T##_t *T = (T##_t*)obj;			\
    dealloc_vec( T->data, T->cap, V );		\
  }

#define ARRAY_RESIZE(T, V, type, padfn)				\
  void resize_##T( T##_t *T, size_t newl)			\
  {								\
    size_t oldc = T->cap;					\
    size_t newc = padfn(newl, T->len, oldc, Mincs[type]);	\
  								\
    if (T->cap != oldc)						\
	T->data = resize_vec(T->data, oldc, newc, V);		\
    T->len = newl;						\
  }

#define ARRAY_WRITE(T, V)				\
  arity32_t T##_write( T##_t *T, V *src, arity32_t n)	\
  {							\
    arity32_t offset = T->len;				\
    resize_##T( T, T->len + n );			\
    memcpy(T->data+offset, src, n * sizeof(V));		\
    return T->len;					\
  }

#define ARRAY_PUSH(T, V)			\
  arity32_t T##_push( T##_t *T, V val)		\
  {						\
    arity32_t offset = T->len;			\
    resize_##T( T, T->len+1 );			\
    T->data[offset] = val;			\
    return offset;				\
  }

#define ARRAY_POP(T, V)				\
  bool T##_pop(T##_t *T, V *buf)		\
  {						\
    if (T->len == 0)				\
      return false;				\
    if (buf)					\
      *buf = T->data[T->len-1];			\
    resize_##T( T, T->len-1 );			\
    return true;				\
  }

#define ARRAY_CLEAR(T, V, type)				\
  void clear_##T(T##_t *T)				\
  {							\
    resize_##T( T, 0 );					\
    memset( T->data, 0, Mincs[type] * sizeof(V) );	\
  }

// table describe macros
#define ENTRY_SLOTS(K, key, V, val)			\
  K        key;						\
  V        val;						\
  hash64_t hash

#define TABLE_INIT(T, E, type)				\
  void init_##T(T##_t *T)				\
  {							\
    init_obj(&T->obj, type, 0);				\
    T->len   = 0;					\
    T->cap   = Mincs[type];				\
    T->data  = alloc_vec(T->cap, E##_t* );		\
  }

#define TABLE_TRACE(T, E)			\
  void trace_##T(obj_t *obj)			\
  {						\
    T##_t *T = (T##_t*)obj;			\
    trace_objs((obj_t**)T->data, T->cap);	\
  }

#define TABLE_FREE(T, E)			\
  void free_##T(obj_t *obj)			\
  {						\
    T##_t* T = (T##_t*)obj;			\
    dealloc_vec(T->data, T->cap, E##_t*);	\
  }

#define TABLE_CLEAR(T, E)				\
  void clear_##T(T##_t *T)				\
  {							\
    resize_##T( T, 0 );					\
    memset( T->data, 0, T->cap * sizeof(E##_t*));	\
  }

#define TABLE_REHASH(T, E)						\
  void rehash_##T(E##_t **old,size_t oldc,E##_t **new,size_t newc)	\
  {									\
    for (arity32_t i=0; i<oldc; i++)					\
      {									\
	E##_t *E = old[i];						\
	if (E == NULL)							\
	  continue;							\
									\
	hash64_t  h = E->hash;						\
	arity32_t m = newc-1;						\
	arity32_t j = h & m;						\
									\
	while (new[j] != NULL)						\
	  j = (j+1) & m;						\
									\
	new[j] = E;							\
      }									\
  }

#define TABLE_RESIZE(T, E, type)				\
  void resize_##T(T##_t *T, size_t newl)			\
  {								\
    size_t oldc = T->cap;					\
    size_t newc = pad_table_size(T->len,newl,oldc,Mincs[type]);	\
    if (newc != oldc)						\
      {								\
	E##_t **oldspace = T->data;				\
	E##_t **newspace = alloc_vec(newc, E##_t*);		\
	rehash_##T(oldspace, oldc, newspace, newc);		\
								\
	T->data = newspace;					\
	T->cap  = newc;						\
								\
	dealloc_vec(oldspace, oldc, E##_t*);			\
      }								\
    T->len = newl;						\
  }

#define TABLE_PUT(T, E, K, key, hashfn, cmpfn)	\
  bool T##_put(T##_t *T, K key, E##_t **buf)	\
  {						\
    hash64_t  h  = hashfn(key);			\
    bool      o  = false;			\
    arity32_t m  = T->cap-1;			\
    arity32_t i  = h & m;			\
    E##_t **E##s = T->data;			\
    E##_t  *E    = NULL;			\
    						\
    while ((E=E##s[i]) != NULL)				\
      {							\
	if (E->hash == h && cmpfn(E->key, key) == 0)	\
	  break;					\
	i = (i+1) & m;					\
      }							\
    							\
    if (E == NULL)					\
      {							\
	o = true;					\
	E##s[i] = E = new_##E();			\
	E->hash = h;					\
	resize_##T(T, T->len+1);			\
      }							\
    if (buf)						\
      *buf = E;						\
    return o;						\
  }

#define TABLE_GET(T, E, K, key, hashfn, cmpfn)		\
  bool T##_get(T##_t *T, K key, E##_t **buf)		\
  {							\
    hash64_t  h  = hashfn(key);				\
    arity32_t m  = T->cap-1;				\
    arity32_t i  = h & m;				\
    E##_t **E##s = T->data;				\
    E##_t  *E    = NULL;				\
							\
    while ((E=E##s[i]) != NULL)				\
      {							\
	if (E->hash == h && cmpfn(E->key, key) == 0)	\
	  break;					\
	i = (i+1) & m;					\
      }							\
    							\
    if (buf)						\
      *buf = E;						\
    return E != NULL;					\
  }

#endif
