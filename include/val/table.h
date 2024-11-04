#ifndef rl_val_table_h
#define rl_val_table_h

#include "val/object.h"

/* Declarations and definitions for mutable and immutable Rascal table types, as well as supporting globals. */
/* C types */

// immutable tables
struct Map {
  HEADER;

  size64  arity;

  union {
    Pair** kvs;
    MNode* root;
  };
};

struct MNode {
  HEADER;

  ushort cnt, cap;
  uint   shift;
  size_t bmap;
  Obj**  cn;
};

struct Table {
  HEADER;

  // data fields
  size64 cnt, cap;
  Pair** kvs;
  int*   map;
};

/* Globals */
// empty singletons
extern Map EmptyMap;

/* APIs */
// map API
#define is_map(x) has_type(x, &MapType)
#define as_map(x) ((Map*)as_obj(x))

Map*  mk_map(size64 n, Val* kvs);
bool  map_get(Map* m, Val k, Val* r);
bool  map_has(Map* m, Val k);
Map*  map_set(Map* m, Val k, Val v);
Map*  map_put(Map* m, Val k, Val v);
Map*  map_pop(Map* m, Val k, Pair** kv);
Map*  join_maps(Map* x, Map* y);

// tables (mutable mappings, used to implement environments)
Table* mk_table(bool rk);
void   init_table(Table* t, bool rk);
bool   table_get(Table* t, Val k, Val* v);
bool   table_set(Table* t, Val k, Val* v);
bool   table_put(Table* t, Val k, Val* v);
bool   table_pop(Table* t, Val k, Pair** kv);
void   join_tables(Table* tx, Table* ty);

#endif
