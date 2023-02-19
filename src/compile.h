#ifndef compile_h
#define compile_h

#include "value.h"

// C types --------------------------------------------------------------------
typedef enum {
  OP_BEGIN,

  OP_LOAD_NIL,
  OP_LOAD_LIST0,
  OP_LOAD_STRING0,

  OP_LOAD_CONSTANT,

  OP_RETURN
} OpCode;

// API ------------------------------------------------------------------------
Func* compile(Val x);

#endif
