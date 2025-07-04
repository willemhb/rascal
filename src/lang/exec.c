/**
 *
 * High level API for entry into the bytecode interpreter.
 * 
 **/

// headers --------------------------------------------------------------------
#include "sys/sys.h"

#include "data/data.h"

#include "lang/exec.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------

Expr exec_code(Fun* fun) {
  void* labels[] = {
    [OP_NOOP]        = &&op_noop,

    // stack manipulation -----------------------------------------------------
    [OP_POP]         = &&op_pop,
    [OP_RPOP]        = &&op_rpop,

    // environment instructions -----------------------------------------------
    [OP_GET_VALUE]   = &&op_get_value,
    [OP_GET_GLOBAL]  = &&op_get_global,
    [OP_SET_GLOBAL]  = &&op_set_global,
    [OP_GET_LOCAL]   = &&op_get_local,
    [OP_SET_LOCAL]   = &&op_set_local,
    [OP_GET_UPVAL]   = &&op_get_upval,
    [OP_SET_UPVAL]   = &&op_set_upval,

    // jump instructions ------------------------------------------------------
    [OP_JUMP]        = &&op_jump,
    [OP_JUMP_F]      = &&op_jump_f,

    // exception interface instructions ---------------------------------------
    [OP_CATCH]       = &&op_catch,
    [OP_THROW]       = &&op_throw,
    [OP_ECATCH]      = &&op_ecatch,

    // function call instructions ---------------------------------------------
    [OP_CLOSURE]     = &&op_closure,
    [OP_CAPTURE]     = &&op_capture,
    [OP_CALL]        = &&op_call,
    [OP_RETURN]      = &&op_return,

    // arithmetic instructions ------------------------------------------------
    [OP_ADD]         = &&op_add,
    [OP_SUB]         = &&op_sub,
    [OP_MUL]         = &&op_mul,
    [OP_DIV]         = &&op_div,

    // miscellaneous builtins -------------------------------------------------
    [OP_EGAL]        = &&op_egal,
    [OP_TYPE]        = &&op_type,

    // sequence operations ----------------------------------------------------
    [OP_CONS]        = &&op_cons,
    [OP_HEAD]        = &&op_head,
    [OP_TAIL]        = &&op_tail,
    [OP_NTH]         = &&op_nth,

    // system instructions ----------------------------------------------------
    [OP_HEAP_REPORT] = &&op_heap_report,
    [OP_DIS]         = &&op_dis,
  };

  int argc, argx, argy;
  OpCode op;
  Expr x, y, z;
  Num nx, ny, nz;
  List* lx, * ly;
  Status sig;

  install_fun(fun, 0);

 fetch:
  op = next_op();

  goto *labels[op];

  // miscellaneous instructions -----------------------------------------------
 op_noop:
  goto fetch;

  // stack manipulation instructions ------------------------------------------
 op_pop: // remove TOS
  vpop();

  goto fetch;

 op_rpop: // move TOS to TOS-1, then decrement sp
  vrpop();

  goto fetch;

  // value/variable instructions ----------------------------------------------
 op_get_value: // load a value from the constant store
  argx = next_op();
  x    = alist_get(Vm.fn->chunk->vals, argx);

  vpush(x);

  goto fetch;

 op_get_global:
  argx = next_op(); // previously resolved index in global environment
  x    = toplevel_env_ref(&Globals, argx);

  require(x != NONE, "undefined reference");
  vpush(x);

  goto fetch;

 op_set_global:
  argx = next_op();
  x    = vpop();

  toplevel_env_refset(&Globals, argx, x);

  goto fetch;

 op_get_local:
  argx = next_op();
  x    = Vm.vals[Vm.bp+argx];

  vpush(x);

  goto fetch;

 op_set_local:
  argx = next_op();
  x    = vpop();
  Vm.vals[Vm.bp+argx] = x;

  goto fetch;

 op_get_upval:
  argx = next_op();
  x    = upval_ref(Vm.fn, argx);

  vpush(x);

  goto fetch;

 op_set_upval:
  argx = next_op();
  x    = vpop();

  upval_set(Vm.fn, argx, x);

  goto fetch;

  // branching instructions ---------------------------------------------------
 op_jump: // unconditional jumping
  argx   = next_op();
  Vm.pc += argx;

  goto fetch;

 op_jump_f: // jump if TOS is falsey
  argx   = next_op();
  x      = vpop();

  if ( is_falsey(x) )
    Vm.pc += argx;

  goto fetch;

  // exception interface instructions -----------------------------------------
 op_catch:
  // handler should be TOS
  // save current execution context
  save_ctx();

  sig = safepoint();

  if ( sig ) {
    // argument to handler
    x = sig > USER_ERROR ? tag_obj(ErrorTypes[sig]) : tos();

    recover(NULL);
    discard_ctx();

    // set up call to handler
    argc  = 1;
    fun   = as_fun(tos());
    tos() = x;

    // jump to handler
    goto call_user_fn;
  }

  // unconditional jump that will skip over
  // the catch body if the handler is invoked
  Vm.pc += 2;

  // enter catch body
  goto fetch;

 op_throw:
  user_error("");

 op_ecatch:
  discard_ctx(); // discard saved context
  vrpop();       // discard handler and leave last expression of catch body at tos

  goto fetch;

  // closures and function calls ----------------------------------------------
 op_call:
  argc = next_op();
  x    = vpop();
  fun  = as_fun_s(Vm.fn->name->val->val, x);

  if ( is_user_fn(fun) )
    goto call_user_fn;

  op = fun->label;

  goto *labels[op];

 call_user_fn:
  require_argco("fn", user_fn_argc(fun), argc);
  save_frame();                                 // save caller state
  install_fun(fun, Vm.sp-argc);

  goto fetch;

 op_closure:
  fun   = as_fun(tos());
  fun   = mk_closure(fun);
  tos() = tag_obj(fun);     // make sure new closure is visible to GC
  argc  = next_op();

  for ( int i=0; i < argc; i++ ) {
    argx = next_op();
    argy = next_op();

    if ( argx == 0 ) // nonlocal
      fun->upvs.vals[i] = Vm.fn->upvs.vals[argy];

    else
      fun->upvs.vals[i] = get_upv(Vm.vals+Vm.bp+argy);
  }

  goto fetch;

  // emmitted before a frame with local upvalues returns
 op_capture:
  close_upvs(Vm.vals+Vm.bp);

  goto fetch;

 op_return:
  x = Vm.sp > 0 ? vpop() : NUL;

  if ( Vm.fp == 0 ) { // no calling frame, exit
    reset_vm();
    return x;
  }

  restore_frame();
  vpush(x);

  goto fetch;

  // builtin functions --------------------------------------------------------
  // at some hypothetical point in --------------------------------------------
  // the future these will be inlineable --------------------------------------
 op_add:
  require_argco("+", 2, argc);

  y      = vpop();
  x      = vpop();
  nx     = as_num_s("+", x);
  ny     = as_num_s("+", y);
  nz     = nx + ny;
  z      = tag_num(nz);

  vpush(z);
  
  goto fetch;

 op_sub:
  require_argco("-", 2, argc);

  y      = vpop();
  x      = vpop();
  nx     = as_num_s("-", x);
  ny     = as_num_s("-", y);
  nz     = nx - ny;
  z      = tag_num(nz);

  vpush(z);

  goto fetch;

 op_mul:
  require_argco("*", 2, argc);

  y      = vpop();
  x      = vpop();
  nx     = as_num_s("*", x);
  ny     = as_num_s("*", y);
  nz     = nx * ny;
  z      = tag_num(nz);

  vpush(z);

  goto fetch;

 op_div:
  require_argco("/", 2, argc);

  y      = vpop();
  x      = vpop();
  nx     = as_num_s("/", x);
  ny     = as_num_s("/", y); require(ny != 0, "division by zero");
  nz     = nx / ny;
  z      = tag_num(nz);

  vpush(z);

  goto fetch;

 op_egal:
  require_argco("=", 2, argc);

  y      = vpop();
  x      = vpop();
  z      = egal_exps(x, y) ? TRUE : FALSE;

  vpush(z);

  goto fetch;

 op_type:
  require_argco("type", 1, argc);

  x      = vpop();
  y      = tag_obj(exp_info(x)->repr);

  vpush(y);

  goto fetch;

 op_cons:
  require_argco("cons", 2, argc);

  lx = as_list_s("cons", Vm.vals[Vm.sp-1]);
  ly = cons(Vm.vals[Vm.sp-2], lx);
  z  = tag_obj(ly);

  vpopn(2);
  vpush(z);

  goto fetch;

 op_head:
  require_argco("head", 1, argc);

  x  = vpop();
  lx = as_list_s("head", x);

  require(lx->count > 0, "can't call head on empty list");

  y  = lx->head;

  vpush(y);

  goto fetch;

 op_tail:
  require_argco("tail", 1, argc);

  x  = vpop();
  lx = as_list_s("tail", x);

  require(lx->count > 0, "can't call tail on empty list");

  ly = lx->tail;

  vpush(tag_obj(ly));

  goto fetch;

 op_nth:
  require_argco("nth", 2, argc);

  x     = vpop();
  argx  = as_num_s("nth", x);
  x     = vpop();
  lx    = as_list_s("nth", x);

  require(argx < (int)lx->count, "index out of bounds");

  x = list_ref(lx, argx);

  vpush(x);

  goto fetch;

 op_heap_report:
  require_argco("*heap-report*", 0, argc);

  heap_report();

  vpush(NUL); // dummy return value

  goto fetch;

 op_dis:
  require_argco("*dis*", 1, argc);
  
  x   = vpop();
  fun = as_fun_s("*dis*", x);

  require(is_user_fn(fun), "can't disassemble builtin function");
  disassemble(fun);

  vpush(NUL);

  goto fetch;
}

// initialization -------------------------------------------------------------
