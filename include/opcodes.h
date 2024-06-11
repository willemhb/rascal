#ifndef rl_opcodes_h
#define rl_opcodes_h

#include "common.h"

typedef enum {
  // constant loads
  OP_LD_NUL,
  OP_LD_TRUE, OP_LD_FALSE,
  OP_LD_ZERO, OP_LD_ONE,

  // register loads
  OP_LD_SLF,              // load currently executing closure
  OP_LD_ENV,              // load current environment

  // common loads/stores
  OP_LD_VAL,             // load from value store
  OP_LD_STK, OP_STO_STK, // load or store from the stack
  OP_LD_UPV, OP_STO_UPV, // load or store from upvalues
  OP_LD_MOD, OP_STO_MOD, // load or store from module

  // handle, raise, catch, throw
  OP_HANDLE, OP_RAISE,   // resumable effects
  OP_CATCH,  OP_THROW,   // non-resumable effects

  // branch instructions
  OP_JMP,                // unconditional jump
  OP_JMP_TRUE,           // conditional jump (pops TOS)
  OP_JMP_FALSE,          // conditional jump (pops TOS)
} OpCode;

#endif
