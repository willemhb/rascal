#include "exec.h"

#include "sym.h"
#include "func.h"
#include "native.h"
#include "prim.h"
#include "module.h"
#include "code.h"
#include "vec.h"
#include "small.h"
#include "list.h"

#include "error.h"
#include "vm.h"

/* API */
/* external */
val_t exec(module_t m) {
  /* just wraps underlying call */
#ifdef DEBUG
  dis_module(m);
#endif

  return exec_at(m, op_begin, 0);
}

val_t exec_at(module_t module, opcode_t entry, uint argx) {
  static void* labels[]  = {
    [op_begin]           = &&label_begin,
    [op_halt]            = &&label_halt,
    [op_noop]            = &&label_noop,
    [op_pop]             = &&label_pop,

    [op_load_nul]        = &&label_load_nul,
    [op_load_small_zero] = &&label_load_small_zero,
    [op_load_small_one]  = &&label_load_small_one,
    [op_load_small_16]   = &&label_load_small_16,
    [op_load_const]      = &&label_load_const,
    [op_load_global]     = &&label_load_global,
    
    [op_invoke]          = &&label_invoke,

    [op_jump]            = &&label_jump,

    [op_save_prompt]     = &&label_save_prompt,
    [op_restore_prompt]  = &&label_restore_prompt,
    [op_discard_prompt]  = &&label_discard_prompt
  };

  opcode_t op=entry;

  size_t argc;

  ushort rx=argx; short sx; int ix;

  val_t x, v, *bp;

  func_err_t status;

  goto *labels[entry];

 label_begin:
  Vm.pc      = 0;
  Vm.program = module;

  goto label_dispatch;
  
 label_halt:
  v = vals_pop(Vm.stack);

  reset_vm(&Vm);

  return v;

 label_noop:
  goto label_dispatch;

 label_pop:
  pop();

  goto label_dispatch;

 label_load_nul:
  push(NUL);

  goto label_dispatch;

 label_load_small_zero:
  push(ZERO);

  goto label_dispatch;

 label_load_small_one:
  push(ONE);

  goto label_dispatch;

 label_load_small_16:
  sx = rx;
  ix = sx;

  push(tag_val(ix, SMALL));

  goto label_dispatch;

 label_load_const:
  v = get_module_const(Vm.program, rx);

  vals_push(Vm.stack, v);

  goto label_dispatch;

 label_load_global:
  x = get_module_const(Vm.program, rx);
  v = sym_head(as_sym(x))->val;

  push(v);

  goto label_dispatch;

 label_invoke:
  bp = peep(-(rx+1));
  x  = (bp++)[0];

  status = validate_func(x, rx, bp);

  if (status != func_no_err) {
    Vm.error = FuncallErrors[status];
    goto label_restore_prompt;
  }

  if (is_prim(x)) {
    goto *labels[as_prim(x)->label];

  } else if (is_native(x)) {
    v = as_native(x)->funptr(rx, bp);
    popn(rx+1);
    push(v);

    goto label_dispatch;

  } else {
    goto label_invoke_module;
  }

 label_jump:
  Vm.pc += rx;

  goto label_dispatch;

 label_save_prompt:
  x = tag_val(Vm.cp, SMALL);
  push(tag_val(Vm.program, OBJECT));
  push(tag_val(Vm.pc, SMALL));
  Vm.cp = push(x);

  goto label_dispatch;

 label_restore_prompt:
  if (Vm.cp == -1) { // toplevel, must fail
    push(Vm.error);
    Vm.error = NUL;

    goto label_halt;
  }

  Vm.error = NUL;

  vals_trim(Vm.stack, Vm.cp+1);

  Vm.cp = as_small(pop());
  Vm.pc = as_small(pop())+2;
  Vm.program = as_module(pop());

  goto label_dispatch;

 label_discard_prompt:
  v = pop();
  Vm.cp = as_small(pop());
  popn(2);
  push(v);

  goto label_dispatch;

 label_dispatch:
  /* bad way to do this, obvious target for improvement */
  if (module == NULL) // jumped in to execute one instruction, jump out here
    goto label_halt;

  op   = Vm.program->bcode[Vm.pc++];
  argc = op_argc(op);

  if (argc > 0)
    rx = Vm.program->bcode[Vm.pc++];

  goto *labels[op];

 label_invoke_module:
  rl_unreachable();
}
