#ifndef compile_h
#define compile_h

#include "object.h"

/* C types */
typedef enum OpCode OpCode;

enum OpCode {
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_GET_GLOBAL,
  OP_DEFINE_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_NONLOCAL,
  OP_SET_NONLOCAL,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_INVOKE,
  OP_CLOSURE,
  OP_RETURN
};

/* API */
UserFn *compile(Val x);

#endif
