#ifndef rl_collection_h
#define rl_collection_h

#include "common.h"
#include "data.h"

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

typedef struct {
  void* key;
  void* val;
} KV;

typedef struct {
  KV* kvs;
  size_t count, max_count;
} Table;

typedef void (*InternFn)(void** spc, void* k, hash_t h);

void init_table(Table* t);
void free_table(Table* t);

#define TABLE_API(K, V, T, type)                    \
  typedef Table T;                                  \
  bool type##_get(Table* t, K k, V* v);             \
  bool type##_set(Table* t, K k, V v);              \
  bool type##_del(Table* t, K k, V* v);             \
  bool type##_intern(Table* t, K k, InternFn ifn);  \
  bool joine_##type##s(Table* tx, Table* ty)

TABLE_API(char*, Obj*, Strings, strings);

#endif
