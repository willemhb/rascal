#ifndef rascal_opcode_h
#define rascal_opcode_h

#include "common.h"

// virtual machine
typedef enum
  {
    /* misc */
    op_invalid       = 0,
    op_halt          = 1,
    op_nothing       = 2,  // noop
    
    /* load/store instructions */
    op_load_const    = 3, // load from constant store

  } opcode_t;

static inline size_t op_argc( opcode_t opcode )
{
  switch (opcode)
    {
    case op_load_const: return 1;
    default:            return 0;
    }
}

#endif
