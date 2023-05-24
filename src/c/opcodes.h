#ifndef opcodes_h
#define opcodes_h

#include "common.h"

// C types ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef enum {
  OP_NOOP,

  OP_LOAD_VALUE,
  OP_LOAD_LOCAL,
  OP_PUT_LOCAL,
  OP_LOAD_GLOBAL,
  OP_PUT_GLOBAL,

  OP_JUMP,
  OP_JUMP_NIL,

  OP_CLOSURE,
  OP_CALL,
  OP_RETURN
} opcode_t;

static inline usize opcode_argc( opcode_t opcode ) {
  switch ( opcode ) {
    case OP_LOAD_VALUE: case OP_LOAD_GLOBAL: case OP_PUT_GLOBAL:
    case OP_JUMP: case OP_JUMP_NIL: case OP_CALL:
      return 1;

    case OP_LOAD_LOCAL: case OP_PUT_LOCAL:
      return 2;

    default:
      return 0;
  }
}

#endif
