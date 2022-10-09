#ifndef rascal_tuple_h
#define rascal_tuple_h

#include "object.h"

struct tuple_t
{
  object_t object;
  value_t  space[0];
};


#endif
