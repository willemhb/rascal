#ifndef rascal_native_h
#define rascal_native_h

#include "obj.h"

typedef struct native_t native_t;

DECL_OBJ(native);

struct native_t
{
  OBJ_HEAD;

  val_t (*native)(val_t *args, arity_t n);
};

#endif
