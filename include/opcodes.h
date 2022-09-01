#ifndef rascal_opcodes_h
#define rascal_opcodes_h

#include "common.h"

typedef enum
{
  OP_HALT=NUM_BUILTINS,

  // stack manipulation instructions ------------------------------------------
  OP_PUSH, OP_POP, OP_DUP,

  // stores and loads ---------------------------------------------------------
  // constant load instructions -----------------------------------------------
  OP_LOAD_NIL, OP_LOAD_TRUE, OP_LOAD_FALSE, OP_LOAD_ZERO, OP_LOAD_ONE,

  // inlined load instructions ------------------------------------------------
  OP_LOAD_SINT16, OP_LOAD_ASCII,

  // common load/store instructions -------------------------------------------
  OP_LOAD_VALUE,

  OP_LOAD_LOCAL, OP_STORE_LOCAL,

  OP_LOAD_UPVALUE, OP_STORE_UPVALUE,

  OP_LOAD_GLOBAL, OP_STORE_GLOBAL,

  // upvalues -----------------------------------------------------------------
  OP_OPEN_UPVALUE, OP_CLOSE_UPVALUE,

  // control flow instructions ------------------------------------------------
  // jumps --------------------------------------------------------------------
  OP_JUMP, OP_JUMP_TRUE, OP_JUMP_FALSE,

  // function calls -----------------------------------------------------------
  OP_CALL, OP_TAIL_CALL, OP_RETURN
  } opcode_t;

#define OP_PAD (OP_RETURN+1)

#define FNARGC					\
  [F_NIL] = 1,					\
    [F_BOOL] = 1,				\
    [F_FLO] = 1,				\
    [F_BIN] = 0,				\
    [F_]

#define OPARGC					\
    [OP_HALT] = 0,				\
    [OP_PUSH] = 1,				\
    [OP_POP] = 0,				\
    [OP_DUP] = 0,				\
    [OP_LOAD_NIL] = 0,				\
    [OP_LOAD_TRUE] = 0,				\
    [OP_LOAD_FALSE] = 0,			\
    [OP_LOAD_ZERO] = 0,				\
    [OP_LOAD_ONE] = 0,				\
    [OP_LOAD_SINT16] = 1,			\
    [OP_LOAD_ASCII] = 1,			\
    [OP_LOAD_VALUE] = 1,			\
    [OP_LOAD_LOCAL] = 1,			\
    [OP_STORE_LOCAL] = 1,			\
    [OP_LOAD_UPVALUE] = 1,			\
    [OP_STORE_UPVALUE] = 1,			\
    [OP_LOAD_GLOBAL] = 1,			\
    [OP_STORE_GLOBAL] = 1,			\
    [OP_OPEN_UPVALUE] = 1,			\
    [OP_CLOSE_UPVALUE] = 1,			\
    [OP_JUMP] = 1,				\
    [OP_JUMP_TRUE] = 1,				\
    [OP_JUMP_FALSE] = 1,			\
    [OP_CALL] = 1,				\
    [OP_TAIL_CALL] = 1,				\
    [OP_RETURN] = 0

#endif
