#ifndef opcodes_h
#define opcodes_h

#include "object.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum {
  // 0-input instructions -----------------------------------------------------
  OP_NOOP,
  OP_START,
  OP_POP,
  OP_CLOSURE, OP_CONTROL,
  OP_RETURN,
  OP_LOADT, OP_LOADF, OP_LOADN,

  // 1-input instructions -----------------------------------------------------
  OP_ARGC, OP_VARGC,
  OP_CALL,
  OP_LOADV,
  OP_LOADGL, OP_PUTGL,
  OP_LOADL, OP_PUTL,
  OP_JUMP, OP_JUMPF,

  // 2-input instructions -----------------------------------------------------
  OP_LOADCL, OP_PUTCL
} opcode_t;

#define N_OPS (OP_PUTCL+1)

static inline usize opcode_argc( opcode_t opcode ) {
  if ( opcode < OP_CALL )
    return 0;

  else if ( opcode < OP_LOADCL )
    return 1;

  else
    return 2;
}

#endif
