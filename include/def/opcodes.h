#ifndef rascal_opcode_h
#define rascal_opcode_h

#include "common.h"

// virtual machine
typedef enum opcode_t
  {
    /* misc */
    op_invalid       = 0,
    op_halt          = 1,
    op_nothing       = 2,  // noop
    op_pop           = 3,
 
    /* load/store instructions */
    // special constant loads
    op_load_nul      = 4,
    op_load_true     = 5,
    op_load_false    = 6,

    // general load/store
    op_load_const    = 7,
    op_load_local    = 8,
    op_store_local   = 9,
    op_load_upvalue  =10,
    op_store_upvalue =11,
    op_load_global   =12,
    op_store_global  =13,
    op_load_toplevel =14,
    op_store_toplevel=15,

    /* control flow */
    /* branching */
    op_jump_true     =12,
    op_jump_false    =13,
    op_jump          =14,

    /* function calls */
    op_invoke        =15,
    op_invoke_generic=16,
    op_invoke_prim   =17,
    op_invoke_native =18,
    op_invoke_module =19,
    op_invoke_effect =20,
    op_return        =21,

    /* closures & upvalues */
    op_load_closure  =22,
    op_capture_upval =23,
    op_close_upval   =24,
 
    /* modules & imports */
    op_load_module   =25,
    op_import_binding=26,

    /* primitive labels */
    op_idp           =256,
    op_id            =257,
    op_not           =258,
    op_typeof        =259,
  } opcode_t;

static inline size_t op_argc( opcode_t opcode )
{
  switch (opcode)
    {
    case op_close_upval:
    case op_load_const ... op_invoke_effect:
      return 1;

    case op_capture_upval:
    case op_import_binding:
      return 2;

    default:
      return 0;
    }
}

#endif
