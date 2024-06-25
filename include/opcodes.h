#ifndef rl_opcodes_h
#define rl_opcodes_h

#include "common.h"

typedef enum {
  // miscellaneous
  OP_NOOP,

  // stack manipulation
  OP_POP,
  OP_DUP,

  // constant loads
  OP_LD_NUL,
  OP_LD_TRUE,
  OP_LD_FALSE,
  OP_LD_ZERO,
  OP_LD_ONE,

  // register loads
  OP_LD_FUN, // load currently executing closure
  OP_LD_ENV, // load current environment
  
  // common loads/stores
  OP_LD_VAL, // load from value store

  OP_LD_STK,
  OP_PUT_STK, // load or store from the stack

  OP_LD_UPV,
  OP_PUT_UPV, // load or store from upvalues
  
  OP_LD_CNS,
  OP_PUT_CNS, // load or store from current namespace

  OP_LD_QNS,
  OP_PUT_QNS, // load or store from qualified namespace

  // initialize instructions (used to initialize final values [also dynamically infers type constraint])
  OP_INIT_STK,
  OP_INIT_UPV,
  OP_INIT_CNS,
  OP_INIT_QNS,

  // handle, raise, catch, throw
  OP_HANDLE,
  OP_RAISE,   // resumable effects

  OP_CATCH,
  OP_THROW,   // non-resumable effects

  // branch instructions
  OP_JMP,  // unconditional jump
  OP_JMPT, // conditional jump (pops TOS)
  OP_JMPF, // conditional jump (pops TOS)

  // closure/upvalue instructions
  OP_CLOSURE, // create a bound copy of the closure at TOS

  // call/return instructions
  OP_CALL0,  // (f)
  OP_CALL1,  // (f x)
  OP_CALL2,  // (f x y)
  OP_CALLN,  // (f ...)

  OP_TCALL0, // (f)      where `f` is in tail position
  OP_TCALL1, // (f x)    where `f` is in tail position
  OP_TCALL2, // (f x y)  where `f` is in tail position
  OP_TCALLN, // (f ...)  where `f` is in tail position

  OP_RETURN
} OpCode;

#endif
