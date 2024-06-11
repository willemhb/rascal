#ifndef rl_table_h
#define rl_table_h

#include "object.h"

/* Declarations and definitions for mutable and immutable Rascal table types, as well as supporting globals. */
/* C types */
// immutable tables
struct Map {
  HEADER;

  // bit fields
  word_t transient : 1;
  word_t fastcmp   : 1;

  // data fields  
  size_t   count;

  union {
    List*    entries; // for small maps (< 16 keys), stored as a sorted map
    MapNode* root;    // larger maps are stored in a HAMT-based structure
  };
};

struct MapNode {
  HEADER;

  // bit fields
  word_t offset    : 6;
  word_t depth     : 4;
  word_t transient : 1;

  // data fields
  size_t   bitmap;

  Object** children;
};

// mutable tables
#define MUTABLE_ENTRY(K, V)                     \
  K key;                                        \
  V val

#define MUTABLE_TABLE(E)                        \
  word_t loadfactor : 5;                        \
  word_t fastcmp    : 1;                        \
                                                \
  E* entries;                                   \
  size_t count, max_count

typedef struct {
  MUTABLE_ENTRY(Value, Value);
} MMEntry;

struct MutMap {
  HEADER;

  MUTABLE_TABLE(MMEntry);
};

// internal table types
typedef struct {
  char*   key;
  String* val;
} SCEntry;

struct StrCache {
  HEADER;

  MUTABLE_TABLE(SCEntry);
};

typedef struct {
  MUTABLE_ENTRY(Symbol*, Binding*);
} EMEntry;

struct EnvMap {
  HEADER;

  MUTABLE_TABLE(EMEntry);
};

#undef MUTABLE_TABLE
#undef MUTABLE_ENTRY

/* Globals */
// types
extern Type MapType, MapNodeType, MutMapType, StrCacheType, EnvMapType;


/* APIs */

#endif
