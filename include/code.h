#ifndef rascal_code_h
#define rascal_code_h

#include "array.h"

// types that represent compiled code go in here
struct instr_t
{
  obj_t obj;
  ARRAY_SLOTS(op_t);
};

struct code_t
{
  obj_t obj;
  instr_t *instr;
  alist_t *constants;
};

#endif
