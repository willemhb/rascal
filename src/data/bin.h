#ifndef bin_h
#define bin_h

#include "base/object.h"

#include "util/hash.h"
#include "util/ctype.h"

/* C types */
struct Bin {
  Obj obj;

  void* array;
};

/* API */
bool val_is_bin(Val x);
bool obj_is_bin(Obj* o);

Bin* val_as_bin(Val x);
Bin* obj_as_bin(Obj* o);

Val bin_tag(Bin* b);
flags32 bin_flags(Bin* b);

#endif
