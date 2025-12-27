#ifndef rl_opcode_h
#define rl_opcode_h

#include "common.h"

// vm labels and a few utilities
typedef enum {
  // miscellaneous instructions
  OP_NOOP,

  // stack manipulation
  OP_POP,
  OP_RPOP,

  // constant loads
  OP_TRUE,
  OP_FALSE,
  OP_NUL,
  OP_EOS,
  OP_ZERO,
  OP_ONE,

  // inlined loads
  OP_GLYPH,
  OP_SMALL,

  // variable lookups
  OP_GET_VALUE,
  OP_GET_GLOBAL,
  OP_SET_GLOBAL,
  OP_ADD_METHOD,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_GET_UPVAL,
  OP_SET_UPVAL,

  // branching instructions
  OP_JUMP,
  OP_JUMP_F,
  OP_PJUMP_F,
  OP_PJUMP_T,

  // closures and function calls
  OP_CLOSURE,
  OP_CAPTURE,
  OP_CALL,
  OP_RETURN,

  // error handling
  OP_CATCH,
  OP_RAISE,
  OP_ECATCH,

  // arithmetic instructions
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_REM,
  OP_NEQ,
  OP_NLT,
  OP_NGT,

  // miscellaneous comparison instructions
  OP_EGAL,
  OP_HASH,
  OP_ISA,
  OP_TYPE,

  // basic list operations
  OP_LIST,
  OP_CONS_2,
  OP_CONS_N,
  OP_HEAD,
  OP_TAIL,
  OP_LIST_REF,
  OP_LIST_LEN,

  // basic string operations
  OP_STR,
  OP_CHARS,
  OP_STR_REF,
  OP_STR_LEN,

  // tuple operations
  OP_TUPLE,
  OP_TUPLE_REF,
  OP_TUPLE_LEN,

  // interpreter builtins
  OP_READ,
  OP_EVAL,
  OP_PRINT,
  OP_REPL,
  OP_APPLY,
  OP_COMPILE,
  OP_EXEC,
  OP_LOAD,

  // environment operations
  OP_DEFINED,
  OP_LOCAL_ENV,
  OP_GLOBAL_ENV,
  
  // system instructions
  OP_HEAP_REPORT,
  OP_STACK_REPORT,
  OP_ENV_REPORT,
  OP_STACK_TRACE,
  OP_DIS,
} OpCode;

int op_arity(OpCode op);
char* op_name(OpCode op);

#endif
