#include "object.h"

// cons api
Value cons( Value car, Value cdr )
{
  (Void)car;
  (Void)cdr;
  
  return NULVAL;
}

// globals
Type TypeType =
  {
    {
      .dtype = &TypeType.obj,
    },

    "type",
    sizeof(Type)
  };

Type ConsType =
  {
    {
      .dtype = &TypeType.obj,
    },

    "cons",
    sizeof(Cons)
  };


