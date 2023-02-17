#ifndef data_vec_h
#define data_vec_h

#include "base/object.h"

/* C types */
struct Vec {
  Obj obj;

  Val* array;
};

/* API */

usize vec_len(Vec* v);
usize vec_size(Vec* v, bool actual);

#endif
