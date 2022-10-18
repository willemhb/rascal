#ifndef rascal_obj_table_h
#define rascal_obj_table_h

#include "obj/object.h"

// general operations on tables & generic table type

#define TABLE					\
  OBJECT					\
  size_t     length;				\
  size_t     capacity;				\
  object_t **data;

typedef struct
{
  TABLE
} table_t;

// table describe macros
#define TABLE_INIT(T, E, type, minc)		\
  void init_##T(T##_t *T)			\
  {						\
    init_obj(&T->obj, type, 0);			\
    init_##E(&T->entries);			\
    init_ords(&T->ords);			\
  }

#define TABLE_FREE(T, E)			\
  void free_##T(object_t *obj)			\
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
    for (size_t i=0; i<oldc; i++)					\
      {									\
	E##_t *E = old[i];						\
	if (E == NULL)							\
	  continue;							\
	hash_t  h = E->hash;						\
	size_t m = newc-1;						\
	size_t j = h & m;						\
	while (new[j] != NULL)						\
	  j = (j+1) & m;						\
	new[j] = E;							\
      }									\
  }

#define TABLE_RESIZE(T, E, type)					\
  void resize_##T(T##_t *T, size_t newl)				\
  {									\
    size_t oldc = T->cap, oldl = T->len;				\
    T->len       = newl;						\
    size_t newc = pad_table_size(oldl, newl, oldc, MinCs[type]);	\
    T->cap       = newc;						\
    if (newc != oldc)							\
      {									\
	E##_t **oldspace = T->data;					\
	E##_t **newspace = alloc_vec(newc, E##_t*);			\
	rehash_##T(oldspace, oldc, newspace, newc);			\
	T->data = newspace;						\
	T->cap  = newc;							\
	dealloc_vec(oldspace, oldc, E##_t*);				\
      }									\
  }

#define TABLE_PUT(T, E, K, key, hashfn, cmpfn)		\
  bool T##_put(T##_t *T, K key, E##_t **buf)		\
  {							\
    hash_t  h  = hashfn(key);				\
    bool    o  = false;					\
    size_t m  = T->cap-1;				\
    size_t i  = h & m;					\
    E##_t **E##s = T->data;				\
    E##_t  *E    = NULL;				\
    while ((E=E##s[i]) != NULL)				\
      {							\
	if (E->hash == h && cmpfn(E->key, key) == 0)	\
	  break;					\
	i = (i+1) & m;					\
      }							\
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
    size_t m  = T->cap-1;				\
    size_t i  = h & m;					\
    E##_t **E##s = T->data;				\
    E##_t  *E    = NULL;				\
    while ((E=E##s[i]) != NULL)				\
      {							\
	if (E->hash == h && cmpfn(E->key, key) == 0)	\
	  break;					\
	i = (i+1) & m;					\
      }							\
    if (buf)						\
      *buf = E;						\
    return E != NULL;					\
  }

// forward declarations
int    resize_table( table_t **table, size_t newl ); // 0 = no change, 1 = resize, 2 = new table
size_t pad_table_size( size_t oldl, size_t newl, size_t oldc, size_t newc );
void   trace_table( object_t *object );

#endif
