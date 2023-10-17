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
  OP_LOADP, OP_PUTP, /* private (with-scoped) */
  OP_LOADG, OP_PUTG, /* global (toplevel scope) */

  /* jump instructions */
  OP_JUMP, OP_JUMPF, OP_JUMPT,

  /* function calls */
  OP_CALL0,  OP_CALL1,  OP_CALL2,  OP_CALLN,
  OP_TCALL0, OP_TCALL1, OP_TCALL2, OP_TCALLN,
  OP_RETURN,

  /* closures */
  OP_CLOSURE,

  /* control (effects) */
  OP_PERFORM,

  /* FFI */
  OP_CCALL,

  /* miscellaneous */
  OP_USE,
} OpCode;

// external API
size_t opCodeArgc(OpCode op);

#endif
