#ifndef table_h
#define table_h

#include "base/object.h"

/* C types */
struct Table {
  Obj obj;

  Val* table;
  int count, cap;

  union {
    sint8*  o8;
    sint16* o16;
    sint32* o32;
    sint64* o64;
  };

  Func* compare;
};

/* globals */
#define NOTFOUND_VAL (UNIT_TAG|1)
#define UNDEF_VAL    (UNIT_TAG|2)
#define UNBOUND_VAL  (UNIT_TAG|3)

/* API */
bool   is_table(Val x);
Table* as_table(Val x);
Val    mk_table(Table* table);

// metadata -------------------------------------------------------------------
bool is_ns(Table* table);

// constructors & memory management -------------------------------------------
Table* new_table(int nargs, Val* args, Func* compare, int fl);
void   init_table(Table* self, int nargs, Val* args, Func* compare, int fl);
void   pad_table(Table* self, int n);

// accessors ------------------------------------------------------------------
Val *table_search(Table* self, Val key);
Val  table_ref(Table* self, Val key);
Val  table_set(Table* self, Val key, Val val);
Val  table_del(Table* self, Val key);
bool table_has(Table* self, Val key);
bool table_add(Table* self, Val key);

// initialization -------------------------------------------------------------
void table_init(void);

#endif
