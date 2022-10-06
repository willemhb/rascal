#ifndef rascal_table_h
#define rascal_table_h

#include "array.h"

// macros, common table utilities, and table types that don't fit elsewhere
typedef union ords_t
{
  int8_t  *o8;
  int16_t *o16;
  int32_t *o32;
} ords_t;

// describe macros
#define TABLE_SLOTS(V)					\
  V          *data;					\
  arity_t   len;					\
  arity_t   cap

#define ORDERED_TABLE_SLOTS(V)				\
  V *data;						\
  arity_t len;						\
  arity_t cap;						\
  ords_t  ords;						\
  arity_t ocap;						\
  arity_t osize

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

#define ORDERED_TABLE_INIT(T, E, type)		\
  void init_##T(T##_t *T)			\
  {						\
    init_obj(&T->obj, type, 0);			\
    T->len = 0;					\
    T->cap = MinCs[type];			\
    init_ords(T->cap, &T->ords, &T->osize);	\
  }

#define TABLE_MARK(T, E)			\
  void mark_##T(obj_t *obj)			\
  {						\
    T##_t *T = (T##_t*)obj;			\
    mark_objs((obj_t**)T->data, T->len);	\
  }

#define TABLE_FREE(T, E)			\
  void free_##T(obj_t *obj)			\
  {						\
    T##_t* T = (T##_t*)obj;			\
    dealloc_vec(T->data, T->cap, E##_t*);	\
  }

#define ORDERED_TABLE_FREE(T, E)		\
  void free_##T(obj_t *obj)			\
  {						\
    T##_t* T = (T##_t*)obj;			\
    dealloc_vec(T->data, T->cap, E##_t*);	\
    free_ords(T->ords, T->ocap, T->osize );	\
  }

#define TABLE_CLEAR(T, E)				\
  void clear_##T(T##_t *T)				\
  {							\
    resize_##T( T, 0 );					\
    memset( T->data, 0, T->cap * sizeof(E##_t*));	\
  }

#define TABLE_REHASH(T, E)						\
  void rehash_##T(E##_t **old,arity_t oldc,E##_t **new,arity_t newc)	\
  {									\
    for (arity_t i=0; i<oldc; i++)					\
      {									\
	E##_t *E = old[i];						\
	if (E == NULL)							\
	  continue;							\
	hash_t  h = E->hash;						\
	arity_t m = newc-1;						\
	arity_t j = h & m;						\
	while (new[j] != NULL)						\
	  j = (j+1) & m;						\
	new[j] = E;							\
      }									\
  }

#define ORDERED_TABLE_REHASH(T, E)					\
  void rehash_##T(E##_t **E##s, arity_t len, arity_t cap, ords_t ords)	\
  {									\
    if (cap <= INT8_MAX)						\
      {									\
	for (arity_t i=0; i<len; i++)					\
	  {								\
	    hash_t  h = E##s[i]->hash;					\
	    arity_t m = cap-1;						\
	    arity_t j = h & m;						\
	    								\
	    while (ords.o8[j] == -1)					\
	      j = (j+1) & m;						\
	    								\
	    ords.o8[j] = i;						\
	  }								\
      }									\
    else if (cap <= INT16_MAX)						\
      {									\
	for (arity_t i=0; i<len; i++)					\
	  {								\
	    hash_t  h = E##s[i]->hash;					\
	    arity_t m = cap-1;						\
	    arity_t j = h & m;						\
	    								\
	    while (ords.o16[j] == -1)					\
	      j = (j+1) & m;						\
	    								\
	    ords.o16[j] = i;						\
	  }								\
      }									\
    else								\
      {									\
	for (arity_t i=0; i<len; i++)					\
	  {								\
	    hash_t  h = E##s[i]->hash;					\
	    arity_t m = cap-1;						\
	    arity_t j = h & m;						\
	    while (ords.o32[j] == -1)					\
	      j = (j+1) & m;						\
	    ords.o32[j] = i;						\
	  }								\
      }									\
  }


#define TABLE_RESIZE(T, E, type)					\
  void resize_##T(T##_t *T, arity_t newl)				\
  {									\
    arity_t oldc = T->cap, oldl = T->len;				\
    T->len       = newl;						\
    arity_t newc = pad_table_size(oldl, newl, oldc, MinCs[type]);	\
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

#define ORDERED_TABLE_RESIZE(T, E, type)				\
  void resize_##T(T##_t *T, arity_t newl)				\
  {									\
    arity_t oldc = T->cap, oldl = T->len;				\
    T->len = newl;							\
    arity_t newc = pad_alist_size(oldl,newl,oldc,MinCs[type]);		\
    if (newc != oldc)							\
      {									\
	T->cap  = newc;							\
	T->data = resize_vec(T->data, oldc, newc, E##_t*);		\
	if (resize_ords( oldl, newl, &T->ocap, MinCs[type], &T->ords, &T->osize )) \
	  rehash_##T( T->data, newl, newc, T->ords );			\
      }									\
  }

#define TABLE_PUT(T, E, K, key, hashfn, cmpfn)		\
  bool T##_put(T##_t *T, K key, E##_t **buf)		\
  {							\
    hash_t  h  = hashfn(key);				\
    bool    o  = false;					\
    arity_t m  = T->cap-1;				\
    arity_t i  = h & m;					\
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

#define ORDERED_TABLE_PUT(T, E, K, key, hashfn, cmpfn)		\
  bool T##_put(T##_t *T, K key, E##_t **buf)			\
  {								\
    hash_t  h  = hashfn(key);					\
    bool    o  = false;						\
    arity_t m  = T->cap-1;					\
    arity_t i  = h & m;						\
    E##_t **E##s = T->data;					\
    E##_t  *E    = NULL;					\
    int32_t ord  = -1;						\
								\
    if (T->osize == 1)						\
      {								\
	int8_t *ords = T->ords.o8;				\
	while ((ord=ords[i]) != -1)				\
	  {							\
	    E = E##s[ord];					\
	    if (E->hash == h && cmpfn(E->key, key)==0)		\
	      break;						\
	    i = (i+1) & m;					\
	  }							\
	if (ord==-1)						\
	  ords[i] = ord = T->len;				\
      }								\
    else if (T->osize == 2)					\
      {								\
	int16_t *ords = T->ords.o16;				\
	while ((ord=ords[i]) != -1)				\
	  {							\
	    E = E##s[ord];					\
	    if (E->hash == h && cmpfn(E->key, key)==0)		\
	      break;						\
	    i = (i+1) & m;					\
	  }							\
	if (ord==-1)						\
	  ords[i] = ord = T->len;				\
      }								\
    else							\
      {								\
	int32_t *ords = T->ords.o32;				\
	while ((ord=ords[i]) != -1)				\
	  {							\
	    E = E##s[ord];					\
	    if (E->hash == h && cmpfn(E->key, key)==0)		\
		break;						\
	    i = (i+1) & m;					\
	  }							\
	if (ord==-1)						\
	  ords[i] = ord = T->len;				\
      }								\
    if ((arity_t)ord==T->len)					\
      {								\
	o = true;						\
	E##s[i] = E = new_##E();				\
	E->hash = h;						\
	resize_##T(T, T->len+1);				\
      }								\
    if (buf)							\
      *buf = E;							\
    return o;							\
  }

#define TABLE_GET(T, E, K, key, hashfn, cmpfn)		\
  bool T##_get(T##_t *T, K key, E##_t **buf)		\
  {							\
    hash_t  h  = hashfn(key);				\
    arity_t m  = T->cap-1;				\
    arity_t i  = h & m;					\
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

#define ORDERED_TABLE_GET(T, E, K, key, hashfn, cmpfn)		\
  bool T##_get(T##_t *T, K key, E##_t **buf)			\
  {								\
    hash_t  h  = hashfn(key);					\
    arity_t m  = T->cap-1;					\
    arity_t i  = h & m;						\
    E##_t **E##s = T->data;					\
    E##_t  *E    = NULL;					\
    int32_t o    = -1;						\
    if (T->osize == 1)						\
      {								\
	int8_t *ords = t->ords.o8;				\
	while ((o=ords[i]) != -1)				\
	  {							\
	    E = E##s[i];					\
	    if (E->hash == h && cmpfn(E->key, key) == 0)	\
	      break;						\
	    i = (i+1) & m; 					\
	  }							\
      }								\
    else if (T->osize == 2)					\
      {								\
	int16_t *ords = t->ords.o16;				\
	while ((o=ords[i]) != -1)				\
	  {							\
	    E = E##s[i];					\
	    if (E->hash == h && cmpfn(E->key, key) == 0)	\
	      break;						\
	    i = (i+1) & m; 					\
	  }							\
      }								\
    else							\
      {								\
	int32_t *ords = t->ords.o32;				\
	while ((o=ords[i]) != -1)				\
	  {							\
	    E = E##s[i];					\
	    if (E->hash == h && cmpfn(E->key, key) == 0)	\
	      break;						\
	    i = (i+1) & m; 					\
	  }							\
      }								\
    if (buf && (ord_t)i != -1)					\
      *buf = E;							\
    return (ord_t)i != -1;					\
  }

// forward declarations
void    init_ords(arity_t ocap, ords_t *ords, arity_t *osize);
void    free_ords(ords_t ords, arity_t ocap, arity_t osize);
bool    resize_ords(arity_t oldl, arity_t newl, arity_t *oldc, arity_t minc, ords_t *ords, arity_t *osize);
arity_t pad_table_size(arity_t oldl, arity_t newl, arity_t oldc, arity_t minc);

#endif
