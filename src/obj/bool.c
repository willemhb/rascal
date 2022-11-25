#include "repr.h"

#include "obj/bool.h"
#include "obj/type.h"


/* commentary */

/* C types */

/* globals */

type_t BoolType =
  {
    { NULL, NUL, &TypeType, sizeof(type_t)+sizeof("bool"), 0, true, false },

    "bool",
    bool_idno,

    sizeof(bool),
    repr_sint32,
    true,
    BOOLEAN,
    NUL,
    
  };
