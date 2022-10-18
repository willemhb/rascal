#ifndef rascal_size_h
#define rascal_size_h

#include "rl/value.h"

// forward declarations
size_t val_size(value_t val);
size_t obj_size(object_t *obj);
size_t repr_size(repr_t repr);

// convenience
#define rl_size(val)					\
  _Generic((val),					\
	   value_t:val_size,				\
	   object_t*:obj_size,				\
	   repr_t:repr_size)(val)

#endif
