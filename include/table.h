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
  typedef void (*T##InternFn)(T* t, T##Kv* kv, K k, void* s, V* b);    \
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
    Obj    obj;                                                         \
    T##Kv* kvs;                                                         \
    void*  ord;                                                         \
    size_t kvCnt;                                                       \
    size_t tsCnt;                                                       \
    size_t ordCap;                                                      \
    size_t kvCap;                                                       \
    T##InternFn intern;                                                 \
    T##HashFn hash;                                                     \
    T##EgalFn egal;                                                     \
  };                                                                    \
                                                                        \
  extern struct Type T##Type

#define TABLE_API(T, K, V, _t)                                          \
  T*     new##T(flags_t fl, T##InternFn i, T##HashFn h, T##EgalFn e);   \
  void   init##T(T* t);                                                 \
  void   free##T(void* p);                                              \
  T##Kv* _t##Find(T* t, T##Kv* kvs, size_t c, K k);                     \
  bool   _t##Intern(T* t, K k, void* s, V* b);                          \
  bool   _t##Add(T* t, K k, V* v);                                      \
  bool   _t##Get(T* t, K k, V* v);                                      \
  bool   _t##Set(T* t, K k, V v);                                       \
  bool   _t##Has(T* t, K k);                                            \
  bool   _t##Del(T* t, K k)

#define ORDERED_TABLE_API(T, K, V, _t)                                  \
  T*     new##T(flags_t fl, T##InternFn i, T##HashFn h, T##EgalFn e);   \
  void   init##T(T* t);                                                 \
  void   free##T(void* p);                                              \
  size_t _t##Arity(T* t);                                               \
  void*  _t##Find(T* t, T##Kv* kvs, void* o, size_t c, K k);            \
  bool   _t##Intern(T* t, K k, void* s, V* b);                          \
  bool   _t##Add(T* t, K k, V* v);                                      \
  bool   _t##Get(T* t, K k, V* v);                                      \
  bool   _t##Set(T* t, K k, V v);                                       \
  bool   _t##Has(T* t, K k);                                            \
  bool   _t##Del(T* t, K k)

typedef enum {
  GLOBAL_NS, PRIVATE_NS, LOCAL_NS, NONLOCAL_NS, STRUCT_NS, RECORD_NS,
} NsType;

TABLE_TYPE(SymbolTable, char*, Symbol*);
TABLE_TYPE(Table, Value, Value);
ORDERED_TABLE_TYPE(NameSpace, Symbol*, Binding*);

// external APIs
TABLE_API(SymbolTable, char*, Symbol*, symbolTable);
TABLE_API(Table, Value, Value, table);
ORDERED_TABLE_API(NameSpace, Symbol*, Binding*, nameSpace);

// utilities
NsType getNsType(NameSpace* ns);

#endif
