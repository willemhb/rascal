#ifndef rl_val_table_h
#define rl_val_table_h

#include "val/object.h"

/* Declarations and definitions for mutable and immutable Rascal table types, as well as supporting globals. */
/* C types */

// general mutable tables
typedef struct TNode {
  Val    key;
  hash64 hash;
  Val    val;
} TNode;

struct Table {
  HEADER;

  // size information
  size64 cnt, cap;

  // actual key/value pairs
  TNode* kvs;
};

// general immutable table
struct Map {
  HEADER;

  // size information
  size64 cnt;
  MNode* root;
};

struct MNode {
  HEADER;
  size16 cnt, cap, shft;
  size64 bm;
  Obj** cn;
};

/* Globals */
extern Map EmptyMap;

/* APIs */
// Table API
#define is_table(x) has_type(x, T_TABLE)
#define as_table(x) ((Table*)as_obj(x))

Table* mk_table(State* vm);
void   free_table(State* vm, void* x);
void   init_table(State* vm, Table* t);
bool   table_get(Table* t, Val k, Val* v);
bool   table_set(Table* t, Val k, Val v);
bool   table_add(Table* t, Val k, TNode** l);
bool   table_del(Table* t, Val k, Val* v);
void   join_tables(Table* tx, Table* ty);

// Map API
#define is_map(x)   has_type(x, T_MAP)
#define as_map(x)   ((Map*)as_obj(x))

// external methods
Map* mk_map(size64 n, Val* kvs);
bool map_get(Map* m, Val k, Val* v);
bool map_has(Map* m, Val k);
Map* map_set(Map* m, Val k, Val v);
Map* map_del(Map* m, Val k);
Map* join_maps(Map* mx, Map* my);

// internal methods
Map* new_map(void);

// MNode API
#define is_mnode(x) has_type(x, T_MNODE)
#define as_mnode(x) ((MNode*)as_obj(x))

MNode* new_mnode(bool s, size64 shft, hash64 h);

// initialization
void rl_toplevel_init_table(void);

#endif
