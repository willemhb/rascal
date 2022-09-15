#ifndef rascal_compiler_h
#define rascal_compiler_h

#include "object.h"

typedef enum
  {
    OP_CONSTANT,
    OP_RETURN,
  } OpCode;

struct Template
{
  OBJ_HEAD;
  Tuple    *values;
  Bytecode *instructions;
};

// forward declarations -------------------------------------------------------

#endif
