#include "compile.h"
#include "opcodes.h"

// module globals -------------------------------------------------------------
const size_t OpcodeArity[OP_RETURN+1] =
  {
    [OP_POP] = 0,

    [OP_LOAD_VALUE] = 1,

    [OP_OPEN_UPVALUE] = 2, [OP_CLOSE_UPVALUE] = 2,

    [OP_JUMP] = 1, [OP_JUMP_TRUE] = 1, [OP_JUMP_FALSE] = 1,
    
    [OP_CALL] = 1, [OP_TAIL_CALL] = 1, [OP_RETURN] = 0,
  };


// forward declarations -------------------------------------------------------
void compile_expression( value_t *code, value_t *vals, value_t *loc, value_t *cap, value_t *clo );

// internal utilities ---------------------------------------------------------

// exports --------------------------------------------------------------------
value_t lisp_compile(value_t x)
{
  
  return NIL;
}
