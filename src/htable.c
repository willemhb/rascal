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

#define ALIST_API(A, X, mincap)                                         \
  void init_##A(TYPE(A)* A) {                                           \
    A->len   = 0;                                                       \
    A->cap   = pad_array_size(0, 0, mincap, 1.0);                       \
    A->array = allocate(A->cap * sizeof(X));                            \
  }                                                                     \
  void free_##A(TYPE(A)* A) {                                           \
    deallocate(A->array, A->cap * sizeof(X));                           \
    init_##A(A);                                                        \
  }                                                                     \
  void resize_##A(TYPE(A)* A, usize len) {                              \
    usize newc = pad_array_size(len, A->cap, mincap, 1.0);              \
    if (newc != A->cap) {                                               \
      A->array = reallocate(A->array,newc*sizeof(X),A->cap*sizeof(X));	\
      A->cap   = newc;                                                  \
    }                                                                   \
  }                                                                     \
  usize A##_push(TYPE(A)* A, X x) {                                     \
    resize_##A(A, A->len+1);                                            \
    A->array[A->len] = x;                                               \
    return A->len++;                                                    \
  }                                                                     \
  X A##_pop(TYPE(A)* A) {                                               \
    assert(A->len > 0);                                                 \
    X out = A->array[--A->len];                                         \
    resize_##A(A, A->len);                                              \
    return out;                                                         \
  }                                                                     \
  X A##_popn(TYPE(A)* A, usize n) {                                     \
    assert(A->len >= n);                                                \
    X out = A->array[A->len-1];                                         \
    resize_##A(A, (A->len -= n));                                       \
    return out;                                                         \
  }                                                                     \
  usize A##_write(TYPE(A)* A, usize n, X* buf) {                        \
    usize off = A->len;                                                 \
    resize_##A(A, (A->len += n));                                       \
    memcpy(A->array+off, buf, n * sizeof(X));                           \
    return off;                                                         \
  }

ALIST_API(bytes, ubyte, 32);
ALIST_API(values, value_t, 8);
ALIST_API(objects, object_t*, 8);
ALIST_API(buffer, char, 512);

// hash table apis ------------------------------------------------------------
static void init_reader_table(void** table, usize cap) {            
  memset(table, 0, cap*2*sizeof(void*));
  
  for (usize i=0; i < cap; i++) {
    *(int*)&table[i*2]       = EOF;
    *(funcptr*)&table[i*2+1] = NULL;
  }
}

static void** reader_locate(htable_t* htable, int key) {		
  uhash h = hash_uword(key);                                               
  uword m = htable->cap-1;                                            
  usize i = h & m;                                                    
  int ikey;                                                         
  while ((ikey=*(int*)&htable->table[i*2]) != EOF) {
    if (key == ikey)
      break;                                                          
    i = (i+1) & m;                                                    
  }                                                                   
  return &htable->table[i*2];
}

static void rehash_reader_table(void** oldt, usize oldc, usize oldl, void** newt, usize newc) {
  init_reader_table(newt, newc);
  usize m = newc-1;
  
  for (usize i=0, n=0; i<oldc && n<oldl; i++) {
    int k = *(int*)&oldt[i*2];

    if (k == EOF)
      continue;

    funcptr v = *(funcptr*)&oldt[i*2+1];
    uhash h = hash_uword(k);
    usize j = h & m;

    while (*(int*)&newt[j*2] != EOF)
      j = (j+1) & m;

    *(int*)&newt[j*2]       = k;
    *(funcptr*)&newt[j*2+1] = v;

    n++;
  }
}

void init_reader(htable_t* htable) {
  htable->count = 0;
  htable->cap   = pad_table_size(0, 0);
  htable->table = allocate(htable->cap * 2 * sizeof(void*));
  init_reader_table(htable->table, htable->cap);
}

void free_reader(htable_t* htable) {
  deallocate(htable->table, htable->cap * 2 * sizeof(void*));
  init_reader(htable);
}

void resize_reader(htable_t* htable, usize n) {
  usize newc = pad_table_size(n, htable->cap);

  if (newc != htable->cap) {
    void** newt = allocate(newc * 2 * sizeof(void*));
    rehash_reader_table(htable->table, htable->cap, htable->count, newt, newc);
    deallocate(htable->table, htable->cap * 2 * sizeof(void*));
    htable->table = newt;
    htable->cap = newc;
  }
}

funcptr reader_get(htable_t* htable, int key) {
  return reader_locate(htable, key)[1];
}

funcptr reader_set(htable_t* htable, int key, funcptr val) {
  resize_reader(htable, htable->count+1);
  
  void** spc = reader_locate(htable, key);

  if (*(int*)spc == EOF) {
    *(int*)spc = key;
    htable->count++;
  }

  funcptr out = spc[1];
  spc[1] = val;
  return out;
}

funcptr reader_del(htable_t* htable, int key) {
  void** spc = reader_locate(htable, key);
  funcptr out = spc[1];
  
  if (*(int*)spc != EOF) {
    *(int*)spc = EOF;
    spc[1] = NULL;
    resize_reader(htable, --htable->count);
  }

  return out;
}
