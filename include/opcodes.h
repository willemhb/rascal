
#ifndef rascal_opcode_h
#define rascal_opcode_h

#include "common.h"

// virtual machine
typedef enum
  {
   op_invalid      = 0,
   op_nothing      = 1,  // noop

   /* load/store instructions */
   op_load_nul     = 2,
   op_load_true    = 3,
   op_load_false   = 4,

   op_load_value   = 5, // load from constant store
   op_load_const   = 6,
   op_load_global  = 7,
   op_store_global = 8,

   op_load_closure = 9,
   op_store_closure=10,

   /* jump instructions */
   op_jump_true    =11,
   op_jump_false   =12,
   op_jump         =13,
   
   /* closure/funcall instructions */

   op_invoke       =14,
   op_return       =15,
   op_closure      =16,

   /* validation */
   op_argco        =17,
   op_vargco       =18,
   
   /* miscellaneous */
   op_halt         =19
  } opcode_t;

static inline size_t op_argc( opcode_t opcode )
{
  switch (opcode)
    {
    case op_invalid ... op_load_false:         return 0;
    case op_load_value ... op_store_global:    return 1;
    case op_load_closure ... op_store_closure: return 2;
    case op_jump_true ... op_invoke:           return 1;
    case op_return ... op_closure:             return 0;
    case op_argco ... op_vargco:               return 1;
    case op_halt:                              return 0;
    }
}

#endif
