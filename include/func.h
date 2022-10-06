#ifndef rascal_func_h
#define rascal_func_h

#include "obj.h"

struct func_t
{
  obj_t    obj;
  atom_t  *name;
  metht_t *metht;
};

#endif
