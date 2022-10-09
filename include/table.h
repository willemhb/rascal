#ifndef rascal_table_h
#define rascal_table_h

#include "array.h"
#include "utils.h"

// C types

// describe macros
#define TABLE_SLOTS(E)					\
  E      entries;					\
  ords_t ords

// table describe macros
#define MAP_SLOTS(K, key, V, val)			\
  K      key;						\
  hash_t hash;						\
  V      val

#define SET_SLOTS(K, key)			\
  K key;					\
  hash_t hash

#define TABLE_INIT(T, E, type, minc)		\
  void init_##T(T##_t *T)			\
  {						\
    init_obj(&T->obj, type, 0);			\
    init_##E(&T->entries);			\
    init_ords(&T->ords);			\
  }

#define TABLE_MARK(T, E)						\
  void mark_##T(object_t *obj)						\
  {									\
    T##_t *T = (T##_t*)obj;						\
    mark_objects((object_t**)T->entries.data, T->entries.length);	\
    									\
  }

#define TABLE_FREE(T, E)			\
  void free_##T(object_t *obj)			\
  {						\
    T##_t* T = (T##_t*)obj;			\
    dealloc_vec(T->data, T->cap, E##_t*);	\
  }

#define ORDERED_TABLE_FREE(T, E)		\
  void free_##T(object_t *obj)			\
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

#define ORDERED_TABLE_REHASH(T, E)					\
  void rehash_##T(E##_t **E##s, size_t len, size_t cap, ords_t ords)	\
  {									\
    if (cap <= INT8_MAX)						\
      {									\
	for (size_t i=0; i<len; i++)					\
	  {								\
	    hash_t  h = E##s[i]->hash;					\
	    size_t m = cap-1;						\
	    size_t j = h & m;						\
	    								\
	    while (ords.o8[j] == -1)					\
	      j = (j+1) & m;						\
	    								\
	    ords.o8[j] = i;						\
	  }								\
      }									\
    else if (cap <= INT16_MAX)						\
      {									\
	for (size_t i=0; i<len; i++)					\
	  {								\
	    hash_t  h = E##s[i]->hash;					\
	    size_t m = cap-1;						\
	    size_t j = h & m;						\
	    								\
	    while (ords.o16[j] == -1)					\
	      j = (j+1) & m;						\
	    								\
	    ords.o16[j] = i;						\
	  }								\
      }									\
    else								\
      {									\
	for (size_t i=0; i<len; i++)					\
	  {								\
	    hash_t  h = E##s[i]->hash;					\
	    size_t m = cap-1;						\
	    size_t j = h & m;						\
	    while (ords.o32[j] == -1)					\
	      j = (j+1) & m;						\
	    ords.o32[j] = i;						\
	  }								\
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

#define ORDERED_TABLE_RESIZE(T, E, type)				\
  void resize_##T(T##_t *T, size_t newl)				\
  {									\
    size_t oldc = T->cap, oldl = T->len;				\
    T->len = newl;							\
    size_t newc = pad_alist_size(oldl,newl,oldc,MinCs[type]);		\
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

#define ORDERED_TABLE_PUT(T, E, K, key, hashfn, cmpfn)		\
  bool T##_put(T##_t *T, K key, E##_t **buf)			\
  {								\
    hash_t  h  = hashfn(key);					\
    bool    o  = false;						\
    size_t m  = T->cap-1;					\
    size_t i  = h & m;						\
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
    if ((size_t)ord==T->len)					\
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

#define ORDERED_TABLE_GET(T, E, K, key, hashfn, cmpfn)		\
  bool T##_get(T##_t *T, K key, E##_t **buf)			\
  {								\
    hash_t  h  = hashfn(key);					\
    size_t m  = T->cap-1;					\
    size_t i  = h & m;						\
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


// globals
extern type_t *DictType, *SetType, *SlotsType, *DictEntryType, *SetEntryType, *SlotType;

// table flags
typedef struct dict_t  dict_t;
typedef struct set_t   set_t;
typedef struct slots_t slots_t;
typedef struct envt_t  envt_t;

typedef struct dict_entry_t  dict_entry_t;
typedef struct set_entry_t   set_entry_t;
typedef struct slot_t        slot_t;
typedef struct var_t         var_t;

typedef struct ords_t
{
  size_t length;
  size_t capacity;

  union
  {
    int8_t  *o8;
    int16_t *o16;
    int32_t *o32;
    int64_t *o64;
  };
} ords_t;

typedef struct dict_entries_t
{
  ARRAY_SLOTS(dict_entry_t*);
} dict_entries_t;

typedef struct set_entries_t
{
  ARRAY_SLOTS(set_entry_t*);
} set_entries_t;

typedef struct slots_entries_t
{
  ARRAY_SLOTS(slot_t*);
} slots_entries_t;

typedef struct envt_entries_t
{
  ARRAY_SLOTS(var_t*);
} envt_entries_t;

struct table_t
{
  object_t    object;
  TABLE_SLOTS(obj_alist_t);
};

struct dict_t
{
  object_t object;
  TABLE_SLOTS(dict_entries_t);
};

struct dict_entry_t
{
  object_t object;
  MAP_SLOTS(value_t, key, value_t, bind);
};

struct set_t
{
  object_t object;
  TABLE_SLOTS(set_entries_t);
};

struct set_entry_t
{
  object_t object;
  SET_SLOTS(value_t, value);
};

struct slots_t
{
  object_t object;
  TABLE_SLOTS(slots_entries_t);
};

struct slot_t
{
  object_t    object;

  MAP_SLOTS(symbol_t*, name, size_t, offset);
  type_t     *type;
  cons_t     *props;
};

struct envt_t
{
  object_t object;
  
};

// forward declarations
void    init_ords(size_t ocap, ords_t *ords, size_t *osize);
void    free_ords(ords_t ords, size_t ocap, size_t osize);
bool    resize_ords(size_t oldl, size_t newl, size_t *oldc, size_t minc, ords_t *ords, size_t *osize);
size_t  pad_table_size(size_t oldl, size_t newl, size_t oldc, size_t minc);

// generic table methods
void    init_table(table_t *table, data_type_t *table_type);
void    resize_table(table_t *table, size_t newl);
void    mark_table(object_t *object);
void    free_table(object_t *object);
void    clear_table(table_t *table);

dict_t *new_dict(void);
bool    dict_get(dict_t *dict, value_t key, dict_entry_t **buf);
bool    dict_put(dict_t *dict, value_t key, dict_entry_t **buf);
bool    dict_pop(dict_t *dict, value_t key, dict_entry_t **buf);
void    dict_merge(dict_t *dict_x, dict_t *dict_y);

set_t  *new_set(void);
bool    set_get(set_t *set, value_t key, set_entry_t **buf);
bool    set_put(set_t *set, value_t key, set_entry_t **buf);
bool    set_pop(set_t *set, value_t key, set_entry_t **buf);
void    set_merge(set_t *set_x, set_t *set_y);

slots_t *new_slots(void);
bool    slots_get(slots_t *slots, symbol_t *key, slot_t **buf);
bool    slots_put(slots_t *slots, symbol_t *key, slot_t **buf);
bool    slots_pop(slots_t *slots, symbol_t *key, slot_t **buf);
void    slots_merge(slots_t *slots_x, slots_t *slots_y);

slot_t *new_slot(void);
slot_t *init_slot( slot_t *slot, symbol_t *name, size_t offset, type_t *type, bool unboxed, bool read, bool write, bool trace );

// convenience
#define as_table(val)     ((table_t*)as_ptr(val))
#define table_length(val) (as_table(val)->entries.length)
#define table_data(val)   (as_table(val)->entries.data)

// initialization
void    table_init( void );

#endif
