
#ifndef rascal_opcode_h
#define rascal_opcode_h

#include "common.h"

// virtual machine
typedef enum
  {
   op_invalid       = 0,
   op_nothing       = 1,  // noop

   /* load/store instructions */
   op_load_nul      = 2,
   op_load_true     = 3,
   op_load_false    = 4,

   op_load_const    = 5, // load from constant store
   op_load_variable = 6,
   op_store_variable= 7,

   /* accessor instructions */
   op_getf          = 8,
   op_setf          = 9,
   op_aref          =10,
   op_aset          =11,

   /* jump instructions */
   op_jump_true     = 8,
   op_jump_false    = 9,
   op_jump          =10,

   /* closure/funcall instructions */
   op_construct     =11,
   op_invoke        =12,
   op_return        =13,
   op_closure       =14,

   /* validation */
   op_argco         =15,
   op_vargco        =16,
   op_argtype       =17,
   
   /* miscellaneous */
   op_halt          =18
  } opcode_t;

static inline size_t op_argc( opcode_t opcode )
{
  switch (opcode)
    {
    case op_invalid ... op_load_false:           return 0;
    case op_load_const:                          return 1;
    case op_load_variable ... op_store_variable: return 2;
    case op_jump_true ... op_invoke:             return 1;
    case op_return ... op_closure:               return 0;
    case op_argco ... op_vargco:                 return 1;
    case op_halt:                                return 0;
    }
}

#endif
