#ifndef rascal_table_h
#define rascal_table_h

#include "array.h"

// describe macros
#define TABLE_SLOTS(V)					\
  V          *data;					\
  arity_t   len;					\
  arity_t   cap

// table describe macros
#define ENTRY_SLOTS(K, key, V, val)			\
  K        key;						\
  V        val;						\
  hash_t hash

#define TABLE_INIT(T, E, type)				\
  void init_##T(T##_t *T)				\
  {							\
    init_obj(&T->obj, type, 0);				\
    T->len   = 0;					\
    T->cap   = MinCs[type];				\
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
    for (arity_t i=0; i<oldc; i++)					\
      {									\
	E##_t *E = old[i];						\
	if (E == NULL)							\
	  continue;							\
									\
	hash_t  h = E->hash;						\
	arity_t m = newc-1;						\
	arity_t j = h & m;						\
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
    size_t newc = pad_table_size(T->len,newl,oldc,MinCs[type]);	\
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
    hash_t  h  = hashfn(key);			\
    bool      o  = false;			\
    arity_t m  = T->cap-1;			\
    arity_t i  = h & m;				\
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
    hash_t  h  = hashfn(key);				\
    arity_t m  = T->cap-1;				\
    arity_t i  = h & m;				\
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

// forward declarations
size_t pad_table_size(size_t oldl, size_t newl, size_t oldc, size_t minc);

#endif
