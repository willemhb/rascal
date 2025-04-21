#ifndef rl_collection_h
#define rl_collection_h

#include "common.h"

// macros for defining array and table types
// API declarations go wherever they make sense, but implementations
// go in collection.c because otherwise the macros are a pain in the ass to debug

// array template
#define ALIST_API(A, X, a)                      \
  typedef struct {                              \
    X* vals;                                    \
    int count, max_count;                       \
  } A;                                          \
                                                \
  void init_##a(A* a);                          \
  void free_##a(A* a);                          \
  void grow_##a(A* a);                          \
  void shrink_##a(A* a);                        \
  void resize_##a(A* a, int n);                 \
  void a##_push(A* a, X x);                     \
  X    a##_pop(A* a);                           \
  void a##_write(A* a, X* xs, int n)

// table template
#define TABLE_API(T, K, V, t)                                    \
  typedef struct {                                               \
    K key;                                                       \
    V val;                                                       \
  } T##KV;                                                       \
                                                                 \
  typedef struct {                                               \
    T##KV* kvs;                                                  \
    int count, max_count;                                        \
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
