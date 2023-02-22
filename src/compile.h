#ifndef compile_h
#define compile_h

#include "value.h"

// C types --------------------------------------------------------------------
typedef enum {
  OP_BEGIN          = 0,

  OP_POP            = 1,

  OP_LOAD_NUL       = 2,

  OP_LOAD_CONST     = 3,
  OP_LOAD_GLOBAL    = 4,
  OP_LOAD_LOCAL     = 5,
  OP_LOAD_CLOSURE   = 6,

  OP_DEF_GLOBAL     = 7,
  OP_DEF_LOCAL      = 8,

  OP_SET_GLOBAL     = 9,
  OP_SET_LOCAL      =10,
  OP_SET_CLOSURE    =11,

  OP_JUMP           =12,
  OP_JUMP_IF_FALSE  =13,

  OP_PROMPT         =14,
  OP_ESCAPE         =15,
  OP_RESTORE        =16,

  OP_CLOSURE        =17,
  OP_INVOKE         =18,

  OP_RETURN         =19
} OpCode;

// API ------------------------------------------------------------------------
Func* compile(Val x);

#endif
