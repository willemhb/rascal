#ifndef rascal_cvalue_h
#define rascal_cvalue_h

#include "array.h"

struct cvalue_t
{
  object_t object;
  type_t   type;
  arity_t  size;
  byte     space[0];
};

typedef struct instr_t
{
  object_t object;
  ARRAY_SLOTS(opcode_t);
} instr_t;

// globals

#endif
