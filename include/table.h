#ifndef rascal_table_h
#define rascal_table_h

#include "object.h"

/* utility table types */
#define TABLE_TYPE(T, K, V)                                             \
  typedef struct {                                                      \
    K key;                                                              \
    V val;                                                              \
  } T##Kv;                                                              \
                                                                        \
  typedef void (*T##InternFn)(T* t, T##Kv* kv, K k, void* s, V* b);     \
  typedef hash_t (*T##HashFn)(K k);                                     \
  typedef bool (*T##EgalFn)(K x, K y);                                  \
                                                                        \
  struct T {                                                            \
    Obj         obj;                                                    \
    T##Kv*      kvs;                                                    \
    size_t      cnt;                                                    \
    size_t      cap;                                                    \
    T##InternFn intern;                                                 \
    T##HashFn   hash;                                                   \
    T##EgalFn   egal;                                                   \
  };                                                                    \
                                                                        \
  extern struct Type T##Type

#define ORDERED_TABLE_TYPE(T, K, V)                                     \
  typedef struct {                                                      \
    K key;                                                              \
    V val;                                                              \
  } T##Kv;                                                              \
                                                                        \
  typedef void (*T##InternFn)(T* t, T##Kv* kvs, K k, void* s, V* b);    \
  typedef hash_t (*T##HashFn)(K k);                                     \
  typedef bool (*T##EgalFn)(K x, K y);                                  \
                                                                        \
  struct T {                                                            \
    Obj         obj;                                                    \
    T##Kv*      kvs;                                                    \
    void*       ord;                                                    \
    size_t      kv_cnt;                                                 \
    size_t      ts_cnt;                                                 \
    size_t      ord_cap;                                                \
    size_t      kv_cap;                                                 \
    T##InternFn intern;                                                 \
    T##HashFn   hash;                                                   \
    T##EgalFn   egal;                                                   \
  };                                                                    \
                                                                        \
  extern struct Type T##Type

#define TABLE_API(T, K, V, _t)                                          \
  T*     new_##_t(flags_t fl, T##InternFn i, T##HashFn h, T##EgalFn e); \
  void   init_##_t(T* t);                                               \
  void   free_##_t(void* p);                                            \
  T##Kv* _t##_find(T* t, T##Kv* kvs, size_t c, K k);                    \
  bool   _t##_intern(T* t, K k, void* s, V* b);                         \
  bool   _t##_add(T* t, K k, V* v);                                     \
  bool   _t##_get(T* t, K k, V* v);                                     \
  bool   _t##_set(T* t, K k, V v);                                      \
  bool   _t##_has(T* t, K k);                                           \
  bool   _t##_del(T* t, K k)

#define ORDERED_TABLE_API(T, K, V, _t)                                  \
  T*     new_##_t(flags_t fl, T##InternFn i, T##HashFn h, T##EgalFn e); \
  void   init_##_t(T* t);                                               \
  void   free_##_t(void* p);                                            \
  size_t _t##_arity(T* t);                                              \
  void*  _t##_find(T* t, T##Kv* kvs, void* o, size_t c, K k);           \
  bool   _t##_intern(T* t, K k, void* s, V* b);                         \
  bool   _t##_add(T* t, K k, V* v);                                     \
  bool   _t##_get(T* t, K k, V* v);                                     \
  bool   _t##_set(T* t, K k, V v);                                      \
  bool   _t##_has(T* t, K k);                                           \
  bool   _t##_del(T* t, K k)

typedef enum {
  GLOBAL_NS, LOCAL_NS, NONLOCAL_NS, STRUCT_NS, RECORD_NS,
} NsType;

TABLE_TYPE(SymbolTable, char*, Symbol*);
TABLE_TYPE(Table, Value, Value);
TABLE_TYPE(Handlers, List*, Chunk*);
ORDERED_TABLE_TYPE(NameSpace, Symbol*, Binding*);

// external APIs
TABLE_API(SymbolTable, char*, Symbol*, symbol_table);
TABLE_API(Table, Value, Value, table);
TABLE_API(Handlers, List*, Chunk*, handlers);
ORDERED_TABLE_API(NameSpace, Symbol*, Binding*, name_space);

// utilities
NsType get_ns_type(NameSpace* ns);

#endif
