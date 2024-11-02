#ifndef rl_val_table_h
#define rl_val_table_h

#include "val/object.h"

/* Declarations and definitions for mutable and immutable Rascal table types, as well as supporting globals. */
/* C types */
// signature for functions used when a new entry needs to be created in a table
typedef void (*InternFn)(void* t, void* e, void* k, void* s, hash_t h);
typedef void (*TInitFn)(void* t, flags32 f);

// immutable tables
struct Map {
  HEADER;
  // bit fields
  flags64 rawkey : 1;   // keys are stored untagged
  flags64 tfmap  : 1;   // this is a set (keys implicitly mapped to true/false)
  
  // data fields
  Struct* type;         // for record types (NULL for vanilla maps)
  size64  arity, size;  // arity is the total entry count, nkvs is the size of tables
  MNode*  root;         // for large persistent maps, all keys are stored in root
  Pair**  kvs, ** last; // kvs is start of entries, last is end of kvs
  int*    mmap;         // mutable mapping (NULL for persistent tables)
};

struct MNode {
  HEADER;

  ushort cnt, cap;
  uint   shift;
  size_t bmap;
  Obj**  cn;
};

/* Globals */
// empty singletons
extern Map EmptyMap;

/* APIs */
// map API
#define is_map(x) has_type(x, &MapType)
#define as_map(x) ((Map*)as_obj(x))

Map*  mk_map(size_t n, Val* kvs);
bool  map_get(Map* m, Val k, Val* r);
bool  map_has(Map* m, Val k);
Map*  map_set(Map* m, Val k, Val v);
Map*  map_put(Map* m, Val k, Val v);
Map*  map_pop(Map* m, Val k, Pair** kv);
Map*  join_maps(Map* x, Map* y);

// mutable maps

#endif
