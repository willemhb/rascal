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
    op_pop           = 3,
    
    /* load/store instructions */
    // special constant loads
    op_load_nul      = 4,

    // general load/store
    op_load_const    = 5,
    op_load_global   = 6,
    op_store_global  = 7,
    op_load_local    = 8,
    op_store_local   = 9,
    op_load_nonlocal =10,
    op_store_nonlocal=11,

    /* control flow */
    op_jump_true     =12,
    op_jump_false    =13,
    op_jump          =14,
    op_invoke        =15,
    op_return        =16,

    /* closures and environments */
    op_make_closure  =17,
    
  } opcode_t;

static inline size_t op_argc( opcode_t opcode )
{
  switch (opcode)
    {
    case op_load_const ... op_store_local:
    case op_jump_true ... op_invoke:
      return 1;

    case op_load_nonlocal:
    case op_store_nonlocal:
      return 2;

    default:
      return 0;
    }
}

#endif
