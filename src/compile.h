#ifndef compile_h
#define compile_h

#include "value.h"

// C types --------------------------------------------------------------------
typedef enum {
  OP_BEGIN               = 0,

  OP_PUSH                = 1,
  OP_DUP                 = 2,
  OP_POP                 = 3,

  OP_LOAD_NUL            = 4,

  OP_LOAD_CONST          = 5,
  OP_LOAD_GLOBAL         = 6,
  OP_LOAD_LOCAL          = 7,
  OP_LOAD_CLOSURE        = 8,

  OP_DEF_GLOBAL          = 9,
  OP_DEF_LOCAL           =10,

  OP_SET_GLOBAL          =11,
  OP_SET_LOCAL           =12,
  OP_SET_CLOSURE         =13,

  OP_JUMP                =14,
  OP_JUMP_IF_FALSE       =15,

  OP_SET_PROMPT          =16,
  OP_REIFY_CONTINUATION  =17,
  OP_INVOKE_EFFECT       =18,
  OP_INVOKE_CONTINUATION =19,

  OP_CAPTURE_CLOSURE     =20,
  OP_INVOKE_FUNCTION     =21,
  OP_TAIL_INVOKE_FUNCTION=22,

  OP_RETURN              =23
} OpCode;

// API ------------------------------------------------------------------------
Func* compile(Val x);

#endif
