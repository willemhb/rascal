#include "exec.h"

#include "sym.h"
#include "func.h"
#include "native.h"
#include "prim.h"
#include "module.h"
#include "code.h"
#include "vec.h"
#include "small.h"
#include "bool.h"
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

#define LOAD_LITERAL_LABEL(_value)		\
  Vm.val = _value;				\
  goto label_dispatch

#define SAVE_FRAME				\
  bp    = vals_pushf(Vm.frame, 6);		\
  bp[0] = tag_val(Vm.bp, SMALL);		\
  bp[1] = tag_val(Vm.fc, SMALL);		\
  bp[2] = tag_val(Vm.cp, SMALL);		\
  bp[3] = tag_val(Vm.pc, SMALL);		\
  bp[4] = tag_val(Vm.pr, OBJECT);		\
  bp[5] = tag_val(Vm.hl, OBJECT)

#define RESTORE_FRAME				\
  bp      = vals_at(Vm.frame, -6);		\
  Vm.bp   = as_small(bp[0]);			\
  Vm.fc   = as_small(bp[1]);			\
  Vm.cp   = as_small(bp[2]);			\
  Vm.pc   = as_small(bp[3]);			\
  Vm.pr   = as_module(bp[4]);			\
  Vm.hl   = as_module(bp[5]);                   \
  vals_trim(Vm.stack, Vm.bp+Vm.fc)

#define UNSAVE_FRAME				\
  vals_popn(Vm.stack, Vm.fc+1);			\
  vals_popn(Vm.frame, 6)
  

  static void* labels[num_opcodes]  = {
    [op_begin]           = &&label_begin,
    [op_halt]            = &&label_halt,
    [op_noop]            = &&label_noop,
    
    [op_push]            = &&label_push,
    [op_pop]             = &&label_pop,
    [op_dup]             = &&label_dup,

    [op_bind]            = &&label_bind,
    [op_bind_vargs]      = &&label_bind_vargs,

    [op_load_nul]        = &&label_load_nul,
    [op_load_true]       = &&label_load_true,
    [op_load_false]      = &&label_load_false,
    [op_load_small_zero] = &&label_load_small_zero,
    [op_load_small_one]  = &&label_load_small_one,
    [op_load_small_16]   = &&label_load_small_16,
    [op_load_const]      = &&label_load_const,
    [op_load_global]     = &&label_load_global,
    [op_store_global]    = &&label_store_global,
    [op_load_local]      = &&label_load_local,
    [op_store_local]     = &&label_store_local,
    [op_load_nonlocal]   = &&label_load_nonlocal,
    [op_store_nonlocal]  = &&label_store_nonlocal,

    [op_invoke]          = &&label_invoke,

    [op_jump]            = &&label_jump,
    [op_jump_true]       = &&label_jump_true,
    [op_jump_false]      = &&label_jump_false,

    [op_save]            = &&label_save,
    [op_unsave]          = &&label_unsave,
    [op_restore]         = &&label_restore,
    
    [op_try]             = &&label_try,
    [op_catch]           = &&label_catch,
    [op_panic]           = &&label_panic
  };

  opcode_t op=entry;

  size_t argc;

  ushort rx=argx, ry; short sx; int ix, iy, iz;

  val_t x, v, *bp;

  cons_t va;

  func_err_t status;

  goto *labels[entry];

 label_begin:
  reset_vm(&Vm);
  Vm.pr = module;

  goto label_dispatch;

 label_halt:
  v = Vm.val;

  reset_vm(&Vm);

  return v;

 label_noop:
  goto label_dispatch;

 label_push:
  push(Vm.val);

  goto label_dispatch;

 label_pop:
  Vm.val = pop();

  goto label_dispatch;

 label_dup:
  x = peek(-1);
  push(x);

  goto label_dispatch;

 label_bind:
  push(Vm.val);
  Vm.fc++;

  goto label_dispatch;

 label_bind_vargs:
  iy = Vm.fc;
  ix = func_head(Vm.pr)->nargs; // calculate number of rest arguments
  
  if (iy > ix) {
    iz = iy - ix;
    va = make_list(iz, peep(-iz));
    x  = tag_val(va, OBJECT);
    vals_popn(Vm.stack, iz);
    Vm.fc = ix;
  } else {
    x = NUL;
  }

  Vm.val = x;

  goto label_bind;

 label_load_nul:        LOAD_LITERAL_LABEL(NUL);
 label_load_true:       LOAD_LITERAL_LABEL(TRUE);
 label_load_false:      LOAD_LITERAL_LABEL(FALSE);
 label_load_small_zero: LOAD_LITERAL_LABEL(ZERO);
 label_load_small_one:  LOAD_LITERAL_LABEL(ONE);

 label_load_small_16:
  sx = rx;
  ix = sx;

  push(tag_val(ix, SMALL));

  goto label_dispatch;

 label_load_const:
  Vm.val = get_module_const(Vm.pr, rx);

  goto label_dispatch;

 label_load_global:
  x      = get_module_const(Vm.pr, rx);
  Vm.val = sym_head(as_sym(x))->val;

  goto label_dispatch;

 label_store_global:
  x = get_module_const(Vm.pr, rx);

  assign(x, Vm.val);

  goto label_dispatch;

 label_load_local:
  Vm.val = peek(Vm.bp + rx);

  goto label_dispatch;

 label_store_local:
  poke(Vm.bp+rx, Vm.val);

  goto label_dispatch;

 label_load_nonlocal:
  iy = Vm.bp;

  while (rx--) {
    x  = vals_ref(Vm.stack, iy-1);
    iy = as_small(x);
  }

  Vm.val = peek(iy+ry);

  goto label_dispatch;

 label_store_nonlocal:
  iy = Vm.bp;

  while (rx--) {
    x  = vals_ref(Vm.stack, iy-1);
    iy = as_small(x);
  }

  poke(iy+ry, Vm.val);

  goto label_dispatch;

 label_invoke:
  bp = peep(-(rx+1));
  x  = (bp++)[0];

  status = validate_func(x, rx, bp);

  if (status != func_no_err) {
    Vm.val = FuncallErrors[status];
    goto label_catch;
  }

  if (is_prim(x))
    goto label_invoke_prim;

  else if (is_native(x))
    goto label_invoke_native;

  else
    goto label_invoke_module;

 label_jump:
  Vm.pc += rx;

  goto label_dispatch;

 label_jump_true:
  if (Cbool(Vm.val))
    Vm.pc += rx;

  goto label_dispatch;

 label_jump_false:
  if (!Cbool(Vm.val))
    Vm.pc += rx;

  goto label_dispatch;

 label_save:
  SAVE_FRAME;
  
  goto label_dispatch;

 label_unsave:
  UNSAVE_FRAME;

  goto label_dispatch;

 label_restore:
  RESTORE_FRAME;

  goto label_dispatch;

 label_try:
  assert(is_module(Vm.val));

  Vm.cp = Vm.frame->len;
  Vm.hl = as_module(Vm.val);

  goto label_dispatch;

 label_catch:
  if (Vm.hl == NULL)
    goto label_panic;

  x  = tag_val(Vm.hl, OBJECT);
  v  = Vm.val;
  rx = 1;

  while (Vm.frame->len > (size_t)Vm.cp) {
    UNSAVE_FRAME;
  }

  RESTORE_FRAME;

  goto label_invoke_module;

 label_panic:
  Vm.error      = Vm.val;
  Vm.panic_mode = true;

  goto label_halt;

 label_dispatch:
  /* bad way to do this, obvious target for improvement */
  if (module == NULL) // jumped in to execute one instruction, jump out here
    goto label_halt;

  op   = Vm.pr->bcode[Vm.pc++];
  argc = op_argc(op);

  if (argc > 0)
    rx = Vm.pr->bcode[Vm.pc++];

  if (argc > 1)
    ry = Vm.pr->bcode[Vm.pc++];

  goto *labels[op];

 label_invoke_prim:
  goto *labels[as_prim(Vm.val)->label];

 label_invoke_native:
  Vm.val     = as_native(x)->funptr(rx, bp);

  vals_popn(Vm.stack, rx+1);

  goto label_dispatch;

 label_invoke_module:
  poke(-rx+1, tag_val(Vm.bp, SMALL));

  Vm.bp      = Vm.stack->len - rx;
  Vm.pc      = 0;
  Vm.pr      = as_module(x);
  Vm.fc      = rx;

  goto label_dispatch;

  #undef LOAD_LITERAL_LABEL
  #undef SAVE_FRAME
  #undef RESTORE_FRAME
  #undef UNSAVE_FRAME
}
