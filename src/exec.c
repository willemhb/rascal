#include "exec.h"
#include "interp.h"
#include "opcode.h"
#include "object.h"

/* local helpers */
static int op_argc(OpCode op) {
  switch (op) {
    case OP_LOADL ... OP_DEFG:
    case OP_JUMP ... OP_INVOKE:
      return 1;

    case OP_LOADN ... OP_STOREN:
      return 2;
      
    default:
      return 0;
  }
}

bool rl_to_C_bool(Val x) {
  return x != NIL_VAL && x != FALSE_VAL;
}

/* API */
Val exec(UserFn *closure) {
  return exec_at(closure, OP_START, 0, 0);
}

Val exec_at(UserFn *closure, OpCode label, uint16 argx, uint16 argy) {
  static void* labels[] = {
    [OP_START]  = &&op_start,  [OP_HALT]   = &&op_halt,
    [OP_LOADC]  = &&op_loadc,  [OP_NIL]    = &&op_nil,
    [OP_TRUE]   = &&op_true,   [OP_FALSE]  = &&op_false,  [OP_POP]   = &&op_pop,
    [OP_LOADL]  = &&op_loadl,  [OP_STOREL] = &&op_storel, [OP_LOADG] = &&op_loadg,
    [OP_STOREG] = &&op_storeg, [OP_DEFG]   = &&op_defg,   [OP_LOADN] = &&op_loadn,
    [OP_STOREN] = &&op_storen, [OP_JUMP]   = &&op_jump,   [OP_JUMPF] = &&op_jumpf,
    
  };

  Frame* frame;
  Val x, y, z, v;
  int argc;

  goto *labels[label];

 fetch:
  label = *(frame->pc++);
  argc = op_argc(label);

  if (argc > 0)
    argx = *(frame->pc++);

  if (argc > 1)
    argy = *(frame->pc++);

  goto *labels[label];
  
 op_start:

  goto fetch;

 op_halt:
  popf();
  v = pop();
  return v;

 op_loadc:
  push(closure->consts->array[argx]);

  goto fetch;

 op_nil:
  push(NIL_VAL);

  goto fetch;

 op_true:
  push(TRUE_VAL);

  goto fetch;

 op_false:
  push(FALSE_VAL);

  goto fetch;

 op_pop:
  v = pop();

  goto fetch;

 op_loadl:
  push(frame->slots[argx]);

  goto fetch;

 op_storel:
  frame->slots[argx] = peek(-1);

  goto fetch;

 op_loadg:
  x = closure->consts->array[argx];

  goto fetch;

 op_jump:
  frame->pc += argx;

  goto fetch;

 op_jumpf:
  x = pop();
  if (!rl_to_C_bool(x))
    frame->pc += argx;

  goto fetch;
}
