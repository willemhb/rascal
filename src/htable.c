#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "htable.h"
#include "number.h"

// globals --------------------------------------------------------------------
#define MIN_CAP    8ul
#define TABLE_LOAD 0.625

// utilities ------------------------------------------------------------------
usize pad_table_size(usize newct, usize oldcap) {
  usize newcap = MAX(oldcap, MIN_CAP);
  if (newct > newcap * TABLE_LOAD) {
    do {
      newcap <<= 1;
    } while (newct > newcap * TABLE_LOAD);
  } else if (newct < (newcap >> 1) * TABLE_LOAD && newcap > MIN_CAP) {
    do {
      newcap >>= 1;
    } while (newct < (newcap >> 1) * TABLE_LOAD && newcap > MIN_CAP);
  }
  return newcap;
}

// API template macro ---------------------------------------------------------
#undef HTABLE_API
#define HTKEY(table, i, ktype) (*(ktype*)((table)+((i)*2)))
#define HTVAL(table, i, vtype) (*(vtype*)((table)+((i)*2+1)))

#define HTABLE_API(type, ktype, vtype, hashk, cmpk, nokey, noval)	\
  static void init_##type##_table(void** table, usize cap) {		\
    for (usize i=0; i < cap; i++) {					\
      HTKEY(table, i, ktype) = nokey;					\
      HTVAL(table, i, vtype) = noval;					\
    }									\
  }									\
  static void** type##_locate(htable_t* htable, ktype key) {		\
    uhash h = hashk(key);						\
    uword m = htable->cap-1;						\
    usize i = h & m;							\
    ktype ikey;								\
    while ((ikey=HTKEY(htable->table, i, ktype)) != nokey) {		\
      if (cmpk(key, ikey))						\
	break;								\
      i = (i+1) & m;							\
    }									\
    return htable->table + i*2;						\
  }									\
  void init_##type(htable_t* htable) {					\
    htable->count = 0;							\
    htable->cap   = pad_table_size(0, 0);				\
    htable->table = ALLOC_S(calloc, MIN_CAP, sizeof(void*));		\
    init_##type##_table(htable->table, MIN_CAP);			\
  }									\
  void free_##type(htable_t* htable) {					\
    free(htable->table);						\
    init_##type(htable);						\
  }									\
  void resize_##type(htable_t* htable, usize count) {			\
    usize newc = pad_table_size(count, htable->cap);			\
    if (newc != htable->cap) {						\
      usize  newm = newc-1;						\
      void** newt = ALLOC_S(calloc, newc, sizeof(void*));		\
      init_##type##_table(newt, newc);					\
      for (usize i=0, n=0; i<htable->cap && n < htable->count; i++) {	\
	ktype k       = HTKEY(htable->table, i, ktype);			\
	if (k == nokey)							\
	  continue;							\
	vtype v   = HTVAL(htable->table, i, vtype);			\
	uhash h   = hashk(k);						\
	usize idx = h & newm;						\
	while (HTKEY(newt, idx, ktype) != nokey)			\
	  idx = (idx+1) & newm;						\
	HTKEY(newt, idx, ktype) = k;					\
	HTVAL(newt, idx, vtype) = v;					\
      }									\
      htable->cap = newc;						\
    }									\
  }									\
  vtype type##_get(htable_t* htable, ktype key) {			\
    void** loc = type##_locate(htable, key);				\
    									\
    return HTVAL(loc, 0, vtype);					\
  }									\
  vtype type##_set(htable_t* htable, ktype key, vtype val) {		\
    resize_##type(htable, htable->count+1);				\
    void** loc = type##_locate(htable, key);				\
    if (HTKEY(loc, 0, ktype) == nokey) {				\
      htable->count++;							\
      HTKEY(loc, 0, ktype) = key;					\
    }									\
    vtype out            = HTVAL(loc, 0, vtype);			\
    HTVAL(loc, 0, vtype) = val;						\
    return out;								\
  }									\
  vtype type##_del(htable_t *htable, ktype key) {			\
    void** loc = type##_locate(htable, key);				\
    vtype out = (vtype)loc[1];						\
    if (HTKEY(loc, 0, ktype) != nokey) {				\
      HTKEY(loc, 0, ktype) = nokey;					\
      HTVAL(loc, 0, vtype) = noval;					\
      resize_##type(htable, --htable->count);				\
    }									\
    return out;								\
  }

#define READT_CMP(x, y) ((x)==(y))

HTABLE_API(reader, int, funcptr, hash_uword, READT_CMP, -1, NULL);

#undef READT_CMP
