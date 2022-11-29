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
    op_load_global   = 4,
    op_store_global  = 5,

    /* control flow */
    op_invoke        = 6
  } opcode_t;

static inline size_t op_argc( opcode_t opcode )
{
  switch (opcode)
    {
    case op_load_const ... op_invoke:
      return 1;

    default:
      return 0;
    }
}

#endif
