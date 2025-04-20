#ifndef rl_collection_h
#define rl_collection_h

#include "common.h"

// utility collection types

typedef struct {
  void** vals;
  int count, max_count;
} Stack;

// Stack APIs
void init_stack(Stack* a);
void free_stack(Stack* a);
void grow_stack(Stack* a);
void shrink_stack(Stack* a);
void stack_push(Stack* a, void* v);
void* stack_pop(Stack* a);
void trace_objs(Stack* a);
void trace_exps(Stack* a);

// object for storing binary data
typedef struct {
  byte_t* vals;
  int count, max_count;
} Binary;

void init_binary(Binary* b);
void free_binary(Binary* b);
void grow_binary(Binary* b);
void shrink_binary(Binary* b);
void resize_binary(Binary* b, int n);
void binary_write(Binary* b, byte_t c);
void binary_write_n(Binary* b, byte_t* cs, int n);

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
