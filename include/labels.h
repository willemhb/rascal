#ifndef rl_labels_h
#define rl_labels_h

#include "common.h"

/* Virtual machine labels (mostly opcodes, but also includes primitive functions and reader/compiler labels) */

typedef enum {
  // miscellaneous
  OP_READY, // initial state
  OP_NOOP,  // do nothing
  OP_ENTER, // default entry point

  // stack manipulation
  OP_POP,  // pop TOS
  OP_DUP,  // duplicate TOS

  // validation
  OP_CHK_TAG,  // check that TOS-1 has the given tag

  // constant loads
  OP_LD_NUL,   // push NUL to TOS
  OP_LD_TRUE,  // push TRUE to TOS
  OP_LD_FALSE, // push FALSE to TOS
  OP_LD_ZERO,  // push ZERO to TOS
  OP_LD_ONE,   // push ONE to TOS

  // register loads
  OP_LD_FUN, // load currently executing closure
  OP_LD_ENV, // load current environment

  // inlined loads
  OP_LD_S16, // load a 16-bit Small that's been inlined in bytecode
  OP_LD_S32, // load a 32-bit Small that's been inlined in bytecode
  OP_LD_G16, // load a 16-bit Glyph that's been inlined in bytecode
  OP_LD_G32, // load a 32-bit Glyph that's been inlined in bytecode

  // common loads/stores
  OP_LD_VAL, // load from value store

  OP_LD_STK,  // 
  OP_PUT_STK, // store into stack variable

  OP_LD_UPV,  // load from upvalues
  OP_PUT_UPV, // store to upvalues

  OP_LD_CNS,  // 
  OP_PUT_CNS, // load or store from current namespace

  OP_LD_QNS,  // load from qualified namespace
  OP_PUT_QNS, // 

  // initialize instructions (used to initialize final values [also dynamically infers type constraint])
  OP_INIT_STK, // 
  OP_INIT_UPV, // 
  OP_INIT_CNS, // 
  OP_INIT_QNS, // 

  // handle, raise, catch, throw
  OP_HANDLE,  // install a frame for resumable effects
  OP_RAISE,   // jump to nearest enclosing resumable effect frame

  OP_CATCH,   // install a frame for a non-resumable effect
  OP_THROW,   // jump to nearest enclosing non-resumable effect frame

  // branch instructions
  OP_JMP,  // unconditional jump
  OP_JMPT, // conditional jump (pops TOS)
  OP_JMPF, // conditional jump (pops TOS)

  // closure/upvalue instructions
  OP_CLOSURE, // create a bound copy of the closure at TOS
  OP_CAPTURE, // close upvalues greater than current BP

  // call/return instructions
  OP_CALL0,  // (f)
  OP_CALL1,  // (f x)
  OP_CALL2,  // (f x y)
  OP_CALLN,  // (f ...)

  OP_TCALL0, // (f)      where `f` is in tail position
  OP_TCALL1, // (f x)    where `f` is in tail position
  OP_TCALL2, // (f x y)  where `f` is in tail position
  OP_TCALLN, // (f ...)  where `f` is in tail position

  OP_RETURN, // restore caller

  // primitive functions
  FN_READ,   // call `read`
  FN_TREAD,  // call `read` in tail position
  FN_COMP,   // call `comp`
  FN_TCOMP,  // call `comp` in tail position
  FN_EXEC,   // call `exec`
  FN_TEXEC,  // call `exec` in tail position
  FN_SAME,   // call `same?`
  FN_TSAME,  // call `same?` in tail position
  FN_EGAL,   // call `=`
  FN_TEGAL,  // call `=` in tail position
  FN_HASH,   // call `hash`
  FN_THASH,  // call `hash` in tail position
  FN_ORD,    // call `ord`
  FN_TORD,   // call `ord` in tail position

  // reader labels
  RL_READ_NUMBER,
} Label;

#endif
