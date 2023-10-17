#ifndef rascal_opcodes_h
#define rascal_opcodes_h

#include "common.h"

typedef enum OpCode {
  /* miscellaneous */
  OP_NOTHING, OP_POP,

  /* constant load/store instructions */
  OP_NUL, OP_TRUE, OP_FALSE, OP_EMPTY_LIST,
  OP_EMPTY_TUPLE, OP_EMPTY_VEC, OP_EMPTY_MAP,
  OP_EMPTY_STR, OP_EMPTY_BITS,

  /* other load/store instructions */
  OP_LOADV,          /* value (constant store) */
  OP_LOADS, OP_PUTS, /* stack (local values) */
  OP_LOADU, OP_PUTU, /* upvalue (captured from surrounding scope) */
  OP_LOADP, OP_PUTP, /* private (with-scoped) */
  OP_LOADG, OP_PUTG, /* global (toplevel scope) */

  /* jump instructions */
  OP_JUMP, OP_JUMPF, OP_JUMPT,

  /* function calls */
  OP_CALL, OP_RETURN,

  /* closures */
  OP_CLOSURE,

  /* */
} OpCode;

// external API
size_t opCodeArgc(OpCode op);

#endif
