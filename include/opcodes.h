#ifndef rascal_opcodes_h
#define rascal_opcodes_h

#include "common.h"

typedef enum OpCode {
  /* miscellaneous */
  OP_NOTHING, OP_POP,

  /* constant loads */
  OP_NUL, OP_TRUE, OP_FALSE, OP_EMPTY_LIST,
  OP_EMPTY_TUPLE, OP_EMPTY_VEC, OP_EMPTY_MAP,
  OP_EMPTY_STR, OP_EMPTY_BITS,  OP_ZERO, OP_ONE,

  /* other load/store instructions */
  OP_LOADI16,
  OP_LOADG16,
  OP_LOADV,          /* value (constant store) */
  OP_LOADS, OP_PUTS, /* stack (local values) */
  OP_LOADU, OP_PUTU, /* upvalue (captured from surrounding scope) */
  OP_LOADG, OP_PUTG, /* global (toplevel scope) */

  /* jump instructions */
  OP_JUMP,  /* unconditional jump. */
  OP_JUMPF, /* jump if top of stack is falsey (discards TOS). */
  OP_JUMPT, /* jump if top of stack is truthy (discards TOS). */

  /* function calls */
  OP_CALL, OP_TCALL, OP_RETURN,

  /* closures */
  OP_CLOSURE,       /* Create a new closure object. */
  OP_CLOSE_UPVALUE, /* Close the `x`th local upvalue (move its value from the stack to the upvalue object). */

  /* control (effects) */
  OP_PROMPT,  /* Set the prompt pointer to the current frame's offset. */
  OP_CONTROL, /* Create a new Control object reifying the top `x` prompt frames.
                 Preserve the top `y` stack values (arguments to effect handlers).
                 Discard everything else in the prompt frame. */

  /* FFI */
  OP_CCALL,  /* perform an FFI call and interpret the result. */
} OpCode;

// external API
size_t opCodeArgc(OpCode op);

#endif
