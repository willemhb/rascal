#ifndef rl_opcodes_h
#define rl_opcodes_h

#include "common.h"

typedef enum OpCode {
  /* miscellaneous */
  OP_NOTHING, OP_POP, OP_DUP, OP_ROTN,

  /* constant loads */
  OP_NUL, OP_TRUE, OP_FALSE,

  /* other load/store instructions */
  OP_GETV,          /* value (constant store) */
  OP_GETL, OP_PUTL, /* stack (local values) */
  OP_GETU, OP_PUTU, /* upvalue (captured from surrounding scope) */
  OP_GETG, OP_PUTG, /* global (toplevel scope) */

  /* multimethods */
  OP_SPECL, OP_SPECU, OP_SPECG,

  /* jump instructions */
  OP_JUMP,  /* unconditional jump. */
  OP_JUMPF, /* jump if top of stack is falsey (discards TOS). */
  OP_JUMPT, /* jump if top of stack is truthy (discards TOS). */

  /* function calls */
  OP_CALL0,  OP_CALL1,  OP_CALL2,  OP_CALLN,
  OP_TCALL0, OP_TCALL1, OP_TCALL2, OP_TCALLN,
  OP_RETURN,

  /* closures */
  OP_CLOSURE,       /* Create a new closure object. */
  OP_CLOSE_UPVAL,

  /* control (effects) */
  OP_PROMPT, OP_CONTROL,

  /* FFI */
  OP_CCALL,  // perform an FFI call and interpret the result.
} OpCode;

// external API
size_t opcode_argc(OpCode op);

#endif
