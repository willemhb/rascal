#ifndef data_vec_h
#define data_vec_h

#include "base/object.h"

/* C types */
struct Vec {
  Obj obj;

  Val* array;
  usize count, cap;
};

/* API */
bool is_vec(Val x);
Vec* as_vec(Val x);
Val tag_vec(Vec* v);

#endif
