#ifndef rl_opcode_h
#define rl_opcode_h

#include "common.h"

// vm labels and a few utilities
typedef enum {
  // miscellaneous instructions
  OP_NOOP,

  // stack manipulation
  OP_POP,

  // variable lookups
  OP_GET_VALUE,
  OP_GET_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_GET_UPVAL,
  OP_SET_UPVAL,

  // arithmetic instructions
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,

  // miscellaneous comparison instructions
  OP_EGAL,
  OP_TYPE,

  // basic list operations
  OP_CONS,
  OP_HEAD,
  OP_TAIL,
  OP_NTH,

  // branching instructions
  OP_JUMP,
  OP_JUMP_F,

  // closures and function calls
  OP_CLOSURE,
  OP_CAPTURE,
  OP_CALL,
  OP_RETURN
} OpCode;

int   op_arity(OpCode op);
char* op_name(OpCode op);

#endif
