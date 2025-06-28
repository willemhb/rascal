#ifndef rl_sys_opcode_h
#define rl_sys_opcode_h

/* Definitions for VM labels and some basic utilities. */

// headers --------------------------------------------------------------------
#include "common.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
// vm labels and a few utilities
typedef enum {
  // miscellaneous instructions
  OP_NOOP,

  // stack manipulation
  OP_POP,
  OP_RPOP,

  // variable lookups
  OP_GET_VALUE,
  OP_GET_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_GET_UPVAL,
  OP_SET_UPVAL,

  // branching instructions
  OP_JUMP,
  OP_JUMP_F,

  // exception interface instructions
  OP_CATCH,  // save execution context and error handler
  OP_THROW,  // jump to last saved execution context
  OP_ECATCH, // discard a saved execution context (because the `catch` expression is exiting normally)

  // closures and function calls
  OP_CLOSURE,
  OP_CAPTURE,
  OP_CALL,
  OP_RETURN,

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

  // system instructions
  OP_HEAP_REPORT,
  OP_DIS,
} OpCode;

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
int   op_arity(OpCode op);
char* op_name(OpCode op);

#endif
