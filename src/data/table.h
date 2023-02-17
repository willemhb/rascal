#ifndef table_h
#define table_h

#include "base/object.h"

#include "util/hash.h"

/* C types */
struct Table {
  Obj obj;

  Val* table;

  union {
    sint8*  o8;
    sint16* o16;
    sint32* o32;
  };

  Func* compare;
};

/* API */
bool is_table(Val x);
Table* as_table(Val x);
Val tag_table(Table* t);

void print_table(Val x, Table* br);
uhash hash_table(Val x, int bound, uhash* acc);
bool equal_tables(Val x, Val y, Table* be);
int compare_tables(Val x, Val y, Table* be);

#endif
