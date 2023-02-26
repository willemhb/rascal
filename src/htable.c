#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "htable.h"
#include "number.h"

// globals --------------------------------------------------------------------
#define MIN_CAP    8ul
#define TABLE_LOAD 0.625

// utilities ------------------------------------------------------------------
usize pad_array_size(usize newct, usize oldcap, usize mincap, double loadf) {
  usize newcap = MAX(oldcap, mincap);
    if (newct > newcap * loadf) {
    do {
      newcap <<= 1;
    } while (newct > newcap * loadf);
  } else if (newct < (newcap >> 1) * loadf && newcap > mincap) {
    do {
      newcap >>= 1;
    } while (newct < (newcap >> 1) * loadf && newcap > mincap);
  }
  return newcap;
}

usize pad_table_size(usize newct, usize oldcap) {
  return pad_array_size(newct, oldcap, MIN_CAP, TABLE_LOAD);
}

// API template macros --------------------------------------------------------
#undef ALIST_API

#define ALIST_API(A, X, mincap)						\
  void init_##A(TYPE(A)* A) {						\
    A->len   = 0;							\
    A->cap   = mincap;							\
    A->array = allocate(mincap*sizeof(X));				\
  }									\
  void free_##A(TYPE(A)* A) {						\
    deallocate(A, A->cap * sizeof(X));					\
    init_##A(A);                                                        \
  }                                                                     \
  void resize_##A(TYPE(A)* A, usize len) {                              \
    usize newc = pad_array_size(len, A->cap, mincap, 1.0);              \
    if (newc != A->cap) {                                               \
      A->array = reallocate(A->array,newc*sizeof(X),A->cap*sizeof(X));  \
      A->cap   = newc;                                                  \
    }                                                                   \
  }                                                                     \
  usize A##_push(TYPE(A)* A, X x) {                                     \
    resize_##A(A, A->len+1);						\
    A->array[A->len] = x;						\
    return A->len++;							\
  }                                                                     \
  X A##_pop(TYPE(A)* A) {						\
    assert(A->len > 0);							\
    X out = A->array[--A->len];						\
    resize_##A(A, A->len);						\
    return out;								\
  }									\
  usize A##_write(TYPE(A)* A, usize n, X* buf) {			\
    usize off = A->len;							\
    resize_##A(A, (A->len += n));					\
    memcpy(A->array+off, buf, n * sizeof(X));				\
    return off;								\
  }

ALIST_API(bytes, ubyte, 32);
ALIST_API(values, value_t, 8);
ALIST_API(objects, object_t*, 8);
ALIST_API(buffer, char, 512);

  
#undef HTABLE_API
#define HTKEY(table, i, ktype) (*(ktype*)((table)+((i)*2)))
#define HTVAL(table, i, vtype) (*(vtype*)((table)+((i)*2+1)))

#define HTABLE_API(type, ktype, vtype, hashk, cmpk, nokey, noval)       \
  static void init_##type##_table(void** table, usize cap) {            \
    for (usize i=0; i < cap; i++) {                                     \
      HTKEY(table, i, ktype) = nokey;                                   \
      HTVAL(table, i, vtype) = noval;                                   \
    }                                                                   \
  }                                                                     \
  static void** type##_locate(htable_t* htable, ktype key) {            \
    uhash h = hashk(key);                                               \
    uword m = htable->cap-1;                                            \
    usize i = h & m;                                                    \
    ktype ikey;                                                         \
    while ((ikey=HTKEY(htable->table, i, ktype)) != nokey) {            \
      if (cmpk(key, ikey))                                              \
        break;                                                          \
      i = (i+1) & m;                                                    \
    }                                                                   \
    return htable->table + i*2;                                         \
  }                                                                     \
  void init_##type(htable_t* htable) {                                  \
    htable->count = 0;                                                  \
    htable->cap   = pad_table_size(0, 0);                               \
    htable->table = ALLOC_S(calloc, MIN_CAP, sizeof(void*));            \
    init_##type##_table(htable->table, MIN_CAP);                        \
  }                                                                     \
  void free_##type(htable_t* htable) {                                  \
    free(htable->table);                                                \
    init_##type(htable);                                                \
  }                                                                     \
  void resize_##type(htable_t* htable, usize count) {                   \
    usize newc = pad_table_size(count, htable->cap);                    \
    if (newc != htable->cap) {                                          \
      usize  newm = newc-1;                                             \
      void** newt = ALLOC_S(calloc, newc, sizeof(void*));               \
      init_##type##_table(newt, newc);                                  \
      for (usize i=0, n=0; i<htable->cap && n < htable->count; i++) {	\
        ktype k       = HTKEY(htable->table, i, ktype);                 \
        if (k == nokey)                                                 \
          continue;                                                     \
        vtype v   = HTVAL(htable->table, i, vtype);                     \
        uhash h   = hashk(k);                                           \
        usize idx = h & newm;                                           \
        while (HTKEY(newt, idx, ktype) != nokey)                        \
          idx = (idx+1) & newm;                                         \
        HTKEY(newt, idx, ktype) = k;                                    \
        HTVAL(newt, idx, vtype) = v;                                    \
      }                                                                 \
      htable->cap = newc;                                               \
    }                                                                   \
  }                                                                     \
  vtype type##_get(htable_t* htable, ktype key) {                       \
    void** loc = type##_locate(htable, key);                            \
                                                                        \
    return HTVAL(loc, 0, vtype);                                        \
  }                                                                     \
  vtype type##_set(htable_t* htable, ktype key, vtype val) {            \
    resize_##type(htable, htable->count+1);                             \
    void** loc = type##_locate(htable, key);                            \
    if (HTKEY(loc, 0, ktype) == nokey) {                                \
      htable->count++;                                                  \
      HTKEY(loc, 0, ktype) = key;                                       \
    }                                                                   \
    vtype out            = HTVAL(loc, 0, vtype);                        \
    HTVAL(loc, 0, vtype) = val;                                         \
    return out;                                                         \
  }                                                                     \
  vtype type##_del(htable_t *htable, ktype key) {                       \
    void** loc = type##_locate(htable, key);                            \
    vtype out = (vtype)loc[1];                                          \
    if (HTKEY(loc, 0, ktype) != nokey) {                                \
      HTKEY(loc, 0, ktype) = nokey;                                     \
      HTVAL(loc, 0, vtype) = noval;                                     \
      resize_##type(htable, --htable->count);                           \
    }                                                                   \
    return out;                                                         \
  }

#define READT_CMP(x, y) ((x)==(y))

HTABLE_API(reader, int, funcptr, hash_uword, READT_CMP, -1, NULL);

#undef READT_CMP
