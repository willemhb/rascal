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
  LF_750 = 0x05,
  LF_875 = 0x06,
} LF;

// signature for functions used when a new entry needs to be created in a table
typedef void (*rl_intern_fn_t)(void* table, void* spc, void* key, void* state);

// immutable tables
struct Map {
  HEADER;

  // bit fields
  word_t offset    : 6;
  word_t depth     : 4;
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
  word_t loadfactor : 5;                        \
                                                \
  E* entries;                                   \
  size_t count, max_count

typedef struct {
  MUTABLE_ENTRY(Val, Val);
} MMEntry;

struct MMap {
  HEADER;

  MUTABLE_TABLE(MMEntry);
};

// internal table types
typedef struct {
  char*   key;
  Str* val;
} SCEntry;

struct SCache {
  HEADER;

  MUTABLE_TABLE(SCEntry);
};

typedef struct {
  MUTABLE_ENTRY(Sym*, Ref*);
} EMEntry;

struct EnvMap {
  HEADER;

  MUTABLE_TABLE(EMEntry);
};

#undef MUTABLE_TABLE
#undef MUTABLE_ENTRY

/* Globals */
// types
extern Type MapType, MNodeType, MMapType, SCacheType, EnvMapType;

/* APIs */
// map API
Map*  new_map(size_t n, Val* kvs, bool t);
Map*  mk_map(size_t n, Val* kvs);
Pair* map_get(Map* m, Val k);
Map*  map_assoc(Map* m, Val k, Val v);
Map*  map_pop(Map* m, Val k, Pair** buf);

// mutable maps
#define MUTABLE_TABLE(T, E, K, V, t, ...)                       \
  T*       new_##t(T* t, byte_t lf __VA_OPT__(,) __VA_ARGS__);  \
  rl_sig_t init_##t(T* t);                                      \
  rl_sig_t free_##t(T* t);                                      \
  rl_sig_t resize_##t(T* t, size_t n);                          \
  rl_sig_t t##_find(T* t, K k, E** r);                          \
  rl_sig_t t##_intern(T* t, K k, rl_intern_fn_t f, void* s);    \
  rl_sig_t t##_get(T* t, K k, V* v);                            \
  rl_sig_t t##_set(T* t, K k, V v);                             \
  rl_sig_t t##_put(T* t, K k, V v);                             \
  rl_sig_t t##_del(T* t, K k);                                  \
  T*       merge_##t##s(T* x, T* y)

MUTABLE_TABLE(MMap, MMEntry, Val, Val, mmap);
MUTABLE_TABLE(SCache, SCEntry, char*, Str*, scache);
MUTABLE_TABLE(EnvMap, EMEntry, Sym*, Ref*, emap, int scope);

#undef MUTABLE_TABLE

#endif
