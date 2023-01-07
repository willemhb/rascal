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
  push(_value);					\
  goto label_dispatch

#define SAVE_FRAME					\
  ix   = vals_pushn(Vm.frame, 4);			\
  bp   = alist_at(Vm.frame, ix, val_t);			\
  bp[0]= tag_val(Vm.pc, SMALL);				\
  bp[1]= tag_val(Vm.bp, SMALL);				\
  bp[2]= tag_val(Vm.cp, SMALL);				\
  bp[3]= tag_val(Vm.stack->len, SMALL)

#define RESTORE_FRAME(_fp)			\
  ix         = _fp;				\
  bp         = alist_at(Vm.frame, ix, val_t);	\
  Vm.pc      = as_small(bp[0]);			\
  Vm.bp      = as_small(bp[1]);			\
  Vm.cp      = as_small(bp[2]);			\
  iy         = as_small(bp[3]);			\
  Vm.program = as_module(peek(Vm.bp-1));	\
  trim_frame(ix);				\
  trim_stack(iy)

  static void* labels[]  = {
    [op_begin]           = &&label_begin,
    [op_halt]            = &&label_halt,
    [op_noop]            = &&label_noop,
    [op_pop]             = &&label_pop,

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

    [op_invoke]          = &&label_invoke,
    [op_return]          = &&label_return,

    [op_jump]            = &&label_jump,
    [op_jump_true]       = &&label_jump_true,
    [op_jump_false]      = &&label_jump_false,

    [op_save_prompt]     = &&label_save_prompt,
    [op_restore_prompt]  = &&label_restore_prompt,
    [op_discard_prompt]  = &&label_discard_prompt
  };

  opcode_t op=entry;

  size_t argc;

  ushort rx=argx; short sx; int ix, iy;

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
  v = get_module_const(Vm.program, rx);

  vals_push(Vm.stack, v);

  goto label_dispatch;

 label_load_global:
  x = get_module_const(Vm.program, rx);
  v = sym_head(as_sym(x))->val;

  push(v);

  goto label_dispatch;

 label_store_global:
  x = get_module_const(Vm.program, rx);

  assign(x, peek(-1));

  goto label_dispatch;

 label_load_local:
  v = peek(Vm.bp + rx);
  push(v);

  goto label_dispatch;

 label_store_local:
  x = peek(-1);
  poke(Vm.bp+rx, x);

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

 label_return:
  v = peek(-1);

  RESTORE_FRAME(Vm.frame->len-4);

  push(v);

  goto label_dispatch;

 label_jump:
  Vm.pc += rx;

  goto label_dispatch;

 label_jump_true:
  x = pop();

  if (Cbool(x))
    Vm.pc += rx;

  goto label_dispatch;

 label_jump_false:
  x = pop();

  if (!Cbool(x))
    Vm.pc += rx;

  goto label_dispatch;

 label_save_prompt:
  SAVE_FRAME;
  Vm.cp = ix;

  goto label_dispatch;

 label_restore_prompt:
  if (Vm.cp == 0) { // toplevel, must fail
    push(Vm.error);
    Vm.error = NUL;

    goto label_halt;
  }

  Vm.error = NUL;

  RESTORE_FRAME(Vm.cp);

  goto label_dispatch;

 label_discard_prompt:
  v = peek(-1);
  RESTORE_FRAME(Vm.cp);
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
  SAVE_FRAME;

  Vm.bp      = Vm.stack->len - rx;
  Vm.pc      = 0;
  Vm.program = as_module(x);

  goto label_dispatch;

  #undef LOAD_LITERAL_LABEL
  #undef SAVE_FRAME
  #undef RESTORE_FRAME
}
