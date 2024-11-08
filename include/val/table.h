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

/* Globals */

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

#endif
