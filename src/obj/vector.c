#include "obj/vector.h"
#include "obj/array.h"
#include "obj/type.h"

#include "vm/memory.h"

#include "tpl/impl/array.h"

/* commentary */

/* C types */

/* globals */
struct type_t VectorType =
  {
    {
      .type=&TypeType.data,
      .size=sizeof(struct type_t)
    },
    {
      .name="vector",
      .vmtype=vmtype_object,
      .obsize=sizeof(struct vector_t),
      .elsize=sizeof(value_t),
      .stringp=false
    }
  };

/* API */
MAKE_ARR(VectorType.data, vector, value_t);
FREE_ARR(vector, value_t);
RESIZE_ARR(vector, value_t);
ARRAY_REF(vector, value_t);
ARRAY_SET(vector, value_t);
ARRAY_ADD(vector, value_t, value_t);
ARRAY_POP(vector, value_t);
