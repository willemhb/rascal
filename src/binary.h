#ifndef rascal_binary_h
#define rascal_binary_h

#include "object.h"
#include "number.h"

// C types --------------------------------------------------------------------
typedef struct
{
  CType Ctype;
  UInt8 inlined   : 4;
  UInt8 shared    : 4;
} BinaryFl;

struct Binary
{
  OBJ_HEAD(BinaryFl);

  union
  {
    UInt8 bytes[0];
    Void *data;
  };
};

typedef struct Binary String;
typedef struct Binary Instructions;

#endif
