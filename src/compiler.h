#ifndef rascal_compiler_h
#define rascal_compiler_h

#include "object.h"

typedef enum
  {
   // stack manipulation ------------------------------------------------------
   OP_PUSH,
   OP_POP,

   // load & store instructions -----------------------------------------------
   OP_CONSTANT,
   OP_LOAD_GLOBAL,
   OP_STORE_GLOBAL,
   OP_LOAD_LOCAL,
   OP_STORE_LOCAL,
   OP_LOAD_UPVALUE,
   OP_STORE_UPVALUE,
   
   // control flow -----------------------------------------------------------
   OP_JUMP,
   OP_JUMP_TRUE,
   OP_JUMP_FALSE,
   OP_CALL,
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
