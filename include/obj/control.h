#ifndef rascal_control_h
#define rascal_control_h

#include "obj/object.h"

struct control_t
{
  object_t       object;

  arity_t        size;     // number of locals
  arity_t        base;     // offset of first argument

  stack_t       *stack;    // arguments stack
  environment_t *envt;     // bound environment
  bytecode_t    *code;     // executing code object
  opcode_t      *prgc;     // program counter
  control_t     *cont;     // immediate caller
  control_t     *cntl;     // continuation prompt (last place where (with ...) form appears)
};

// globals
extern type_t  *ControlType;
extern dtype_t  ControlTypeOb;

#endif
