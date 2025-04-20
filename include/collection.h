#ifndef rl_collection_h
#define rl_collection_h

#include "common.h"

// utility collection types

typedef struct {
  void** vals;
  size_t count, max_count;
} Alist;

// Alist APIs
void init_alist(Alist* a);
void free_alist(Alist* a);
void grow_alist(Alist* a);
void shrink_alist(Alist* a);
void alist_push(Alist* a, void* v);
void* alist_pop(Alist* a);

// table template
#define TABLE_API(T, K, V, t)                                    \
  typedef struct {                                               \
    K key;                                                       \
    V val;                                                       \
  } T##KV;                                                       \
                                                                 \
  typedef struct {                                               \
    T##KV* kvs;                                                  \
    size_t count, max_count;                                     \
  } T;                                                           \
                                                                 \
  typedef void (*T##InternFn)(T* t, T##KV* kv, K k, hash_t h);   \
                                                                 \
  void init_##t(T* t);                                           \
  void free_##t(T* t);                                           \
  bool t##_get(T* t, K k, V* v);                                 \
  bool t##_set(T* t, K k, V v);                                  \
  bool t##_del(T* t, K k, V* v);                                 \
  V    t##_intern(T* t, K k, T##InternFn ifn)

#endif
