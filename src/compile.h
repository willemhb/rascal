#ifndef compile_h
#define compile_h

#include "value.h"

// C types --------------------------------------------------------------------
typedef enum {
  OP_BEGIN,

  OP_LOAD_NUL,

  OP_LOAD_CONST,
  OP_LOAD_GLOBAL,
  OP_LOAD_LOCAL,
  OP_LOAD_CLOSURE,

  OP_RETURN
} OpCode;

// API ------------------------------------------------------------------------
Func* compile(Val x);

#endif
