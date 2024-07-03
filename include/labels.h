#ifndef rl_labels_h
#define rl_labels_h

#include "common.h"

/* Virtual machine labels (mostly opcodes, but also includes primitive functions, special forms, and reader/compiler labels) */

typedef enum {
  // initial state
  LB_READY,

  // special forms (first so that they fit into Sym bit fields)
  F_CATCH,    // (catch [& a] h b)
  F_DEF,      // (def* x y) | (def* x m y)
  F_DO,       // (do x & r)
  F_GENFN,    // (genfn* f ss) | (genfn* n m ss)
  F_HNDL,     // (hndl [& a] h b)
  F_IF,       // (if t? t) | (if t? t e)
  F_LMB,      // (lmb* [& a] b)
  F_METHOD,   // (method* f s b)
  F_NS,       // (ns n & body) | (ns n m & body)
  F_PUT,      // (put* n v)
  F_QUOTE,    // (quote x)
  F_RAISE,    // (raise o) | (raise k o) | (raise k o a)
  F_THROW,    // (throw e) | (throw e m) | (throw e m b)
  F_TYPE,     // (type* T k s) | (type* T m k s)
  F_USE,      // (use [& nss])

  // miscellaneous
  OP_NOOP,  // do nothing
  OP_ENTER, // default entry point

  // stack manipulation
  OP_POP,  // pop TOS
  OP_DUP,  // duplicate TOS

  // constant loads
  OP_LD_NUL,   // push NUL
  OP_LD_TRUE,  // push TRUE
  OP_LD_FALSE, // push FALSE
  OP_LD_ZERO,  // push ZERO
  OP_LD_ONE,   // push ONE

  // register loads
  OP_LD_FUN, // load currently executing function
  OP_LD_ENV, // load current environment

  // inlined loads
  OP_LD_S16, // load a 16-bit Small that's been inlined in bytecode
  OP_LD_S32, // load a 32-bit Small that's been inlined in bytecode
  OP_LD_G16, // load a 16-bit Glyph that's been inlined in bytecode
  OP_LD_G32, // load a 32-bit Glyph that's been inlined in bytecode

  // common loads/stores
  OP_LD_VAL,   // load from value store
  OP_LD_STK,   // load from stack allocated variable
  OP_PUT_STK,  // store into stack allocated variable
  OP_LD_UPV,   // load from upvalues
  OP_PUT_UPV,  // store to upvalues
  OP_LD_CNS,   // load from current namespace
  OP_PUT_CNS,  // store into current namespace
  OP_LD_QNS,   // load from qualified namespace
  OP_PUT_QNS,  // store into qualified namespace

  // entry point for generic function specialization
  OP_ADDM,     // add_method(P, TOS-2, TOS-1)

  // nonlocal control constructs
  OP_HNDL,     // install a frame for resumable effects
  OP_RAISE1,   // jump to nearest enclosing resumable effect frame (no k or argument supplied)
  OP_RAISE2,   // jump to nearest enclosing resumable effect frame (no k supplied)
  OP_RAISE3,   // jump to nearest enclosing resumable effect frame (all arguments supplied)

  OP_CATCH,   // install a frame for a non-resumable effect
  OP_THROW1,  // jump to nearest enclosing non-resumable effect frame (no msg or blame supplied)
  OP_THROW2,  // jump to nearest enclosing non-resumable effect frame (no blame supplied)
  OP_THROW3,  // jump to nearest enclosing non-resumable effect frame (all arguments supplied)

  // branch instructions
  OP_JMP,  // unconditional jump
  OP_JMPT, // conditional jump (pops TOS)
  OP_JMPF, // conditional jump (pops TOS)

  // closure/upvalue instructions
  OP_CLOSURE, // create a bound copy of the closure at TOS
  OP_CAPTURE, // close upvalues greater than current BP

  // call/return instructions (there's lots of these)
  OP_CALL0,  // (f)
  OP_CALL1,  // (f x)
  OP_CALL2,  // (f x y)
  OP_CALLN,  // (f ...)

  OP_TCALL0, // (f)      where `f` is in tail position
  OP_TCALL1, // (f x)    where `f` is in tail position
  OP_TCALL2, // (f x y)  where `f` is in tail position
  OP_TCALLN, // (f ...)  where `f` is in tail position

  // exit/return/cleanup instructions
  OP_RETURN, // restore caller
  OP_EXIT,   // exit the virtual machine

  // primfn labels
  FN_SAME,   // call `same?`
} Label;

#endif
