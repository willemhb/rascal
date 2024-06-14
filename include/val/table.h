#ifndef rl_val_table_h
#define rl_val_table_h

#include "val/object.h"

/* Declarations and definitions for mutable and immutable Rascal table types, as well as supporting globals. */
/* C types */
// encodes mutable hash table load factors
typedef enum {
  LF_125 = 0x01,
  LF_250 = 0x02,
  LF_375 = 0x03,
  LF_500 = 0x04,
  LF_625 = 0x05,
  LF_750 = 0x06,
  LF_875 = 0x07,
} LF;

// signature for functions used when a new entry needs to be created in a table
typedef void (*rl_intern_fn_t)(void* t, void* e, void* k, void* s, hash_t h);
typedef void (*rl_tinit_fn_t)(void* t, flags_t f);

// immutable tables
struct Map {
  HEADER;

  // bit fields
  word_t offset    : 6;
  word_t transient : 1;

  // data fields  
  size_t   count;

  union {
    List*  entries; // for small maps (< 16 keys), stored as a sorted map
    MNode* root;    // larger maps are stored in a HAMT-based structure
  };
};

struct MNode {
  HEADER;

  // bit fields
  word_t offset    : 6;
  word_t transient : 1;

  // data fields
  size_t bitmap;

  Obj**  children;
};

// mutable tables
#define MUTABLE_ENTRY(K, V)                     \
  K key;                                        \
  V val

#define MUTABLE_TABLE(E)                        \
  word_t lf : 5;                                \
                                                \
  E* entries;                                   \
  size_t cnt, maxc, nts

typedef struct {
  MUTABLE_ENTRY(Val, Val);
} MMEntry;

struct MMap {
  HEADER;

  MUTABLE_TABLE(MMEntry);
};

// internal table types
typedef struct {
  char* key;
  Str*  val;
} SCEntry;

struct SCache {
  HEADER;

  MUTABLE_TABLE(SCEntry);
};

typedef struct {
  MUTABLE_ENTRY(Sym*, Ref*);
} EMEntry;

struct EMap {
  HEADER;

  word_t scope : 3;

  MUTABLE_TABLE(EMEntry);
};

#undef MUTABLE_TABLE
#undef MUTABLE_ENTRY

/* Globals */
// types
extern Type MapType, MNodeType, MMapType, SCacheType, EMapType;

/* APIs */
// map API
#define is_map(x) has_type(x, &MapType)
#define as_map(x) ((Map*)as_obj(x))

Map*  new_map(size_t n, Val* kvs, bool t);
Map*  mk_map(size_t n, Val* kvs);
Pair* map_find(Map* m, Val k);
Val   map_get(Map* m, Val k);
Map*  map_set(Map* m, Val k, Val v);
Map*  map_put(Map* m, Val k, Val v);
Map*  map_pop(Map* m, Val k, Pair** kv);
Map*  join_maps(Map* x, Map* y);

// mutable maps
#define MUTABLE_TABLE(T, E, K, V, t)                                \
  T*       new_##t(LF l, flags_t f);                                \
  void     init_##t(T* t, LF l, flags_t f);                         \
  void     free_##t(void* x);                                       \
  E*       t##_find(T* t, K k);                                     \
  E*       t##_intern(T* t, K k, rl_intern_fn_t f, void* s);        \
  bool     t##_get(T* t, K k, V* r);                                \
  bool     t##_has(T* t, K k);                                      \
  bool     t##_set(T* t, K k, V v, V* r);                           \
  bool     t##_put(T* t, K k, V v);                                 \
  bool     t##_del(T* t, K k, V* r);                                \
  T*       join_##t##s(T* x, T* y)

MUTABLE_TABLE(MMap, MMEntry, Val, Val, mmap);
MUTABLE_TABLE(SCache, SCEntry, char*, Str*, scache);
MUTABLE_TABLE(EMap, EMEntry, Sym*, Ref*, emap);

#undef MUTABLE_TABLE

#endif
