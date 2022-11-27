#include "vmtype.h"
#include "rascal.h"


/* commentary */

/* C types */

/* globals */
vmtype_spec_t TypeSpecs[] =
  {
    [vmtype_nulptr] = { "nulptr", sizeof(nullptr_t), alignof(nullptr_t), numtype_unsigned, 0 },
    [vmtype_flo64]  = { "float-64", sizeof(double), alignof(double),     numtype_float,    0 },
    [vmtype_stream] = { "stream", sizeof(stream_t), alignof(stream_t),   numtype_unsigned, 0 },
    [vmtype_object] = { "object", sizeof(object_t), alignof(object_t),   numtype_unsigned, 0 },
    [vmtype_array]  = { "array", sizeof(array_t),   alignof(array_t),    numtype_unsigned, 0 }
  };

/* API */

/* convenience */
