#include "eval.h"

#include "memory.h"
#include "object.h"
#include "opcodes.h"

// static helpers -------------------------------------------------------------
static void push_frame(uint_t bp, uint_t pc)
{
  Fp += 2;

  
}

value_t lisp_eval(value_t code)
{
  static size_t argcos[OP_PAD] =
    {
      OPARGC
    };

  static void *labels[OP_PAD] =
    {
      // builtin functions (pseudo-opcodes) -----------------------------------
      // constructors ---------------------------------------------------------
      [F_NIL] = &&f_nil, [F_BOOL] = &&f_bool, [F_FLO] = &&f_flo,
      [F_BIN] = &&f_bin, [F_TUP] = &&f_tup, [F_MAP] = &&f_map,
      [F_CLO] = &&f_clo, [F_CONS] = &&f_cons, [F_SYM] = &&f_sym,

      // type predicates ------------------------------------------------------
      [F_NIL_P] = &&f_nil_p, [F_BOOL_P] = &&f_bool_p, [F_FLO_P] = &&f_flo_p,
      [F_BIN_P] = &&f_bin_p, [F_TUP_P] = &&f_tup_p, [F_MAP_P] = &&f_map_p,
      [F_CLO_P] = &&f_clo_p, [F_CONS_P] = &&f_cons_p, [F_SYM_P] = &&f_sym_p,

      // type-ish predicates --------------------------------------------------
      [F_KW_P] = &&f_kw_p, [F_GEN_P] = &&f_gen_p, [F_FUN_P] = &&f_fun_p,
      [F_TYPE_P] = &&f_type_p,

      // other booleans -------------------------------------------------------
      [F_ID_P] = &&f_id_p, [F_NOT] = &&f_not,

      // other utilities ------------------------------------------------------
      [F_ORD] = &&f_ord, [F_HASH] = &&f_hash, [F_SIZE] = &&f_size,

      // arithmetic -----------------------------------------------------------

      // opcodes --------------------------------------------------------------
      // start/stop instructions ----------------------------------------------
      [OP_HALT] = &&op_halt,

      // stack manipulation ---------------------------------------------------
      [OP_PUSH] = &&op_push, [OP_POP] = &&op_pop, [OP_DUP] = &&op_dup,

      // load/store instructions ----------------------------------------------
      // constant loads -------------------------------------------------------
      [OP_LOAD_NIL] = &&op_load_nil, [OP_LOAD_TRUE] = &&op_load_true,
      [OP_LOAD_FALSE] = &&op_load_false, [OP_LOAD_ZERO] = &&op_load_zero,
      [OP_LOAD_ONE] = &&op_load_one,

      // inline loads for small integers --------------------------------------
      [OP_LOAD_SINT16] = &&op_load_sint16, [OP_LOAD_ASCII] = &&op_load_ascii,

      // common load instructions ---------------------------------------------
      [OP_LOAD_VALUE] = &&op_load_value,
      [OP_LOAD_LOCAL] = &&op_load_local, [OP_STORE_LOCAL] = &&op_store_local,
      [OP_LOAD_GLOBAL] = &&op_load_global, [OP_STORE_GLOBAL] = &&op_store_global,
      [OP_LOAD_UPVALUE] = &&op_load_upvalue, [OP_STORE_UPVALUE] = &&op_store_upvalue,

      // control flow ---------------------------------------------------------
      [OP_JUMP] = &&op_jump, [OP_JUMP_TRUE] = &&op_jump_true, [OP_JUMP_FALSE] = &&op_jump_false,
      
      // function calls -------------------------------------------------------
      [OP_CALL] = &&op_call, [OP_TAIL_CALL] = &&op_tail_call, [OP_RETURN] = &&op_return
    };

  value_t x, v = NIL;

  bytecode_t *instr; tuple_t *vals, *envt; value_t upvals;

  size_t argc;
  
  ushort_t op; short_t argx, argy;

  uint_t pc = 0, bp = 0;

  push(code);

  instr = pval(clo_code(code));
  vals = pval(clo_vals(code));
  envt = pval(clo_envt(code));
  
 do_dispatch:
  op   = instr->space[pc++];
  argc = argcos[op];

  if (argc > 1)
    argx = instr->space[pc++];

  if (argc > 2)
    argy = instr->space[pc++];

  goto *labels[op];

  // builtins -----------------------------------------------------------------
  // constructors -------------------------------------------------------------
 f_nil:
  
  goto op_load_nil;

 f_bool:
  x = pop();

  if (x == NIL || x == FALSE)
    goto op_load_false;

  goto op_load_true;

  // type predicates ----------------------------------------------------------
 f_nil_p:
  if (Tos == NIL)
    Tos = TRUE;

  else
    Tos = FALSE;

  goto do_dispatch;

  
  
  
  // opcodes ------------------------------------------------------------------
 op_halt:
  v = pop();

  return v;

 op_push:
 op_pop:
  pop();
  goto do_dispatch;

 op_dup:
  dup();
  goto do_dispatch;

 op_load_nil:
  push(NIL);
  goto do_dispatch;

 op_load_true:
  push(TRUE);
  goto do_dispatch;

 op_load_false:
  push(FALSE);
  goto do_dispatch;

 op_load_zero:
  push(ZERO);
  goto do_dispatch;

 op_load_one:
  push(ONE);
  goto do_dispatch;

 op_load_ascii:
 op_load_sint16:
  x = INTEGER | argx;
  push(x);
  goto do_dispatch;

 op_load_value:
  x = vals->space[argx];
  push(x);
  goto do_dispatch;

 op_load_local:
  x = Stack[Sp-argx];
  push(x);
  goto do_dispatch;

 op_store_local:
  x = Tos;
  Stack[Sp-argx] = x;
  goto do_dispatch;

 op_load_upvalue:
  x = envt->space[argx];

  if (car(x) == FALSE)
    {
      argx = ival(cdr(x));
      goto op_load_local;
    }
  
  x = cdr(x);
  push(x);
  goto do_dispatch;

 op_store_upvalue:
  x = envt->space[argx];

  if (car(x) == FALSE)
    {
      argx = ival(cdr(x));
      goto op_store_local;
    }

  cdr(x) = Tos;
  goto do_dispatch;

 op_load_global:
  x = vals->space[argx];
  v = sym_bind(x);
  push(v);
  goto do_dispatch;

 op_store_global:
  x = vals->space[argx];
  sym_bind(x) = Tos;
  goto do_dispatch;

 op_open_upvalue:

  goto do_dispatch;
  
 op_close_upvalue:

  goto do_dispatch;

 op_jump:
  pc += argx;
  goto do_dispatch;

 op_jump_true:
  x = pop();

  if (x != NIL && x != FALSE)
    pc += argx;

  goto do_dispatch;

 op_jump_false:
  x = pop();

  if (x == NIL || x == FALSE)
    pc += argx;

  goto do_dispatch;

 op_call:
  x = Stack[Sp-1-argx];

 op_tail_call:
  x = Stack[Sp-1-argx];

 op_return:
  v = pop();
  
  popn(Sp-bp);
  
}
