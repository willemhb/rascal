#ifndef data_tuple_h
#define data_tuple_h

#include "base/object.h"

/* C types */
struct Tuple {
  Obj obj;
  Val slots[];
};

/* API */
bool is_tuple(Val x);
Tuple* as_tuple(Val x);
Val tag_tuple(Tuple* t);

#endif
