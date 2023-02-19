#ifndef types_table_h
#define types_table_h

#include "types/object.h"

/* C types */
typedef struct {
  Val key;
  Val val;
} TableEntry;

struct Table {
  Obj *next;
  Type type;
  uint flags  : 27; 
  uint init   :  1;
  uint eql    :  1;
  uint nofree :  1;
  uint black  :  1;
  uint gray   :  1;

  TableEntry *table;
  int *ord;
  uint count, cap;
};

/* API */
// constructors & memory management -------------------------------------------
Table* new_table(bool eql, uint n, Val* args);
void   init_table(Table* self, bool eql, uint n, Val* args);
void   resize_table(Table* self, uint n);

// accessors ------------------------------------------------------------------
int* table_lookup(Table* self, Val key);
Val* table_nth(Table* self, uint n);
Val  table_get(Table* self, Val key);
Val  table_set(Table* self, Val key, Val val);
Val  table_del(Table* self, Val key);

// initialization -------------------------------------------------------------
void table_init(void);

#endif
