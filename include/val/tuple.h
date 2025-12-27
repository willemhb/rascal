#ifndef rl_val_tuple_h
#define rl_val_tuple_h

#include "val/val.h"

// C types --------------------------------------------------------------------
struct Tuple {
  HEAD;
  Expr* data;
  int count;
};

// Globals --------------------------------------------------------------------
extern Type TupleType;

// Prototypes -----------------------------------------------------------------
Tuple* mk_tuple(RlState* rls, int argc);
Tuple* mk_tuple_s(RlState* rls, int argc);

// Helpers/Macros -------------------------------------------------------------
#define tuple_data(t)      ((t)->data)
#define tuple_count(t)     ((t)->count)
#define is_tuple(x)        has_type(x, &TupleType)
#define as_tuple(x)        ((Tuple*)as_obj(x))
#define as_tuple_s(rls, x) ((Tuple*)as_obj_s(rls, &TupleType, x))

#endif
