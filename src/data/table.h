#ifndef table_h
#define table_h

#include "base/object.h"

#include "util/hash.h"

/* C types */
struct Table {
  Obj obj;

  Val* table;
  usize count, cap;

  union {
    sint8*  o8;
    sint16* o16;
    sint32* o32;
    sint64* o64;
  };

  Func* compare;
};

/* API */
bool is_table(Val x);
Table* as_table(Val x);
Val tag_table(Table* t);



#endif
