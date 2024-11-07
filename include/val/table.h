#ifndef rl_val_table_h
#define rl_val_table_h

#include "val/object.h"

/* Declarations and definitions for mutable and immutable Rascal table types, as well as supporting globals. */
/* C types */

// general mutable tables
struct Table {
  HEADER;

  // size information
  size64 kcnt, kcap, mcnt, mcap;

  // actual key/value pairs
  Pair** kvs;

  // mapping from 
  void*  map;
};

/* Globals */

/* APIs */
// Table API
#define is_table(x) has_type(x, T_TABLE)
#define as_table(x) ((Table*)as_obj(x))

// tables (mutable mappings, used to implement environments)
Table* mk_table(void);
void   init_table(Table* t);
bool   table_get(Table* t, Val k, Val* v);
bool   table_set(Table* t, Val k, Val* v);
bool   table_put(Table* t, Val k, Val* v);
bool   table_pop(Table* t, Val k, Pair** kv);
void   join_tables(Table* tx, Table* ty);

#endif
