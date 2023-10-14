#ifndef rascal_opcodes_h
#define rascal_opcodes_h

#include "common.h"

typedef enum OpCode {
  OP_NOTHING, // no-op
  OP_POP,     // discard TOS
  OP_RETURN,  // leave executing chunk
  OP_NUL,     // load `nul`
  OP_TRUE,    // load `true`
  OP_FALSE,   // load `false`
  OP_EMPTY,   // load `()`
  OP_VALUE,   // load value from constant store
  OP_CALL,    // call a function
  OP_JUMP,    // unconditional jump
  OP_JUMPF,   // conditional jump (jump if TOS is `false` or `nul`)
  OP_JUMPT,   // conditional jump (jump if TOS is not `false` or `nul`)
  OP_GETGL,   // load value of global variable with offset `i`
  OP_PUTGL,   // store TOS in global variable with offset `i`
  OP_USE,     // 
} OpCode;

// external API
size_t opCodeArgc(OpCode op);

#endif
