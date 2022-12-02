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
    op_load_variable = 6,
    op_store_variable= 7,

    /* control flow */
    /* branching */
    op_jump_true     = 8,
    op_jump_false    = 9,
    op_jump          =10,

    /* function calls & closures */
    op_invoke        =11,
    op_return        =12,

    /* closures and environments */
    op_make_closure  =13,
  } opcode_t;

static inline size_t op_argc( opcode_t opcode )
{
  switch (opcode)
    {
    case op_load_const:
    case op_jump_true ... op_invoke:
      return 1;

    case op_load_variable:
    case op_store_variable:
      return 2;

    default:
      return 0;
    }
}

#endif
