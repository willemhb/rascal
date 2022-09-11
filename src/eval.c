#include <assert.h>

#include "eval.h"

#include "memory.h"
#include "runtime.h"
#include "array.h"


static const size_t OpArgc[num_instructions] =
  {
   [op_load_const]   = 1,

   [op_load_local]   = 1, [op_store_local]   = 1,
   [op_load_closure] = 2, [op_store_closure] = 2,
   [op_load_global]  = 1, [op_store_global]  = 1,

   [op_jump]         = 1, [op_jump_true]     = 1,
   [op_jump_false]   = 1,

   [op_call]         = 1, [op_return]        = 1
  };

size_t op_argc(instruction_t o)
{
  return OpArgc[o];
}

void push_frame(void)
{
  object_t *saveFn = Fn;

  uint savePc = Pc, saveBp = Bp, saveFp = Fp;

  // save current VM state
  stack_push(Stack, tagp( saveFn, FUNCTION ) );
  stack_push(Stack, tagc( savePc, C_uint32 ) );
  stack_push(Stack, tagc( saveBp, C_uint32 ) );
  stack_push(Stack, tagc( saveFp, C_uint32 ) );

  // update frame pointer
  Fp = Vp;
}

bool pop_frame(void)
{
  if (Fp == 0)
    return false;
  
  value_t tmp;

  // remove arguments to current function
  Vp = Bp;

  // remove saved values from stack
  Sp = Fp;

  tmp = stack_pop(Stack); Fp = uval(tmp);
  tmp = stack_pop(Stack); Bp = uval(tmp);
  tmp = stack_pop(Stack); Pc = uval(tmp);
  tmp = stack_pop(Stack); Fn = pval(tmp);

  resize_stack( Stack );
  resize_stack( Values );

  return Fp > 0;
}

bool catch_unwind(void)
{
  bool caught = false;
  
  while (Fp != 0 && !(caught=is_catch(Frame[-1])))
    pop_frame();

  if (caught)
    {
      // clear flag
      Frame[-1] &= ~((value_t)exec_fl_catch<<32);
      
      if (Saved != Catch)
	{
	  Saved = Catch;
	  Catch = Saved->catch;
	}
    }

  return caught;
}

value_t exec(function_t *code)
{
  static void* labels[num_instructions] =
    {
      [op_push]   = &&do_op_push,   [op_pop] = &&do_op_pop, [op_dup] = &&do_op_dup,
      [op_halt]   = &&do_op_halt,

      [op_load_const] = &&do_op_load_const, 
    };

  instruction_t op;
  int argx, argy, argc;

  // save state on entry (these should all be in initial state but im covering my bases)
  push_frame();

  Fn = (object_t*)code;
  Pc = 0;

  if (setjmp(Toplevel))
    {
      if (catch_unwind())
	goto do_dispatch;

      else
	goto do_op_halt;
    }

 do_dispatch:
  op   = fn_code(Fn)[Pc++];
  argc = op_argc(op);

  if (argc > 0)
    argx = fn_code(Fn)[Pc++];

  if (argc > 1)
    argy = fn_code(Fn)[Pc++];

  goto *labels[op];

 do_op_push:
  stack_push(Values, Val);
  goto do_dispatch;

 do_op_pop:
  stack_pop(Values);
  goto do_dispatch;

 do_op_dup:
  stack_dup(Values);
  goto do_dispatch;

 do_op_load_const:
  Val = fn_constants(Fn)->data[argx];
  goto do_dispatch;

 do_op_catch:
  
  
 do_op_halt:
  return Val;
}
