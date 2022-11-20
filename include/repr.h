#ifndef rascal_repr_h
#define rascal_repr_h

#include "common.h"

// low-level types understood by the VM
typedef enum
  {
    VoidRepr        = 0,
    Int32Repr       = 1,
    BoolRepr        = 2,
    AsciiRepr       = 3,
    PointerRepr     = 4,
    AsciiStringRepr = 5,
    DoubleRepr      = 6,
    UInt64Repr      = 7,
    ObjectRepr      = 8,
  } Repr;

#endif
