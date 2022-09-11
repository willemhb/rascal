#include "eval.h"

static const size_t OpArgc[num_instructions] =
  {
   [op_load_const]   = 1,
   [op_load_local]   = 1, [op_store_local]   = 1,
   [op_load_closure] = 2, [op_store_closure] = 2,
   [op_load_global]  = 1, [op_store_global]  = 1,

   [op_jump]         = 1, [op_jump_true]     = 1,
   [op_jump_false]   = 1,

   [op_call]         = 1,
  };

static size_t op_argc(opcode_t o)
{
  return OpArgc[o];
}

value_t exec(function_t *code)
{
  static void* labels[num_instructions] =
    {
     [op_halt]   = &&do_op_halt
    };
  
 do_op_halt:
  return Val;
}
