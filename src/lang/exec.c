#include <dlfcn.h>

#include "lang.h"
#include "val.h"
#include "val/ffi.h"
#include "vm.h"

// Globals --------------------------------------------------------------------
enum {
  EF_TCALL = 0x01, // don't save caller
};
// Prototypes -----------------------------------------------------------------
// Helpers
bool is_falsey(Expr x);
Expr do_load(RlState* rls, char* fname);

// Implementations ------------------------------------------------------------
// Helpers
bool is_falsey(Expr x) {
  return x == NONE || x == NUL || x == FALSE;
}

// External
Expr load_file(RlState* rls, char* fname) {
  // wrapper that pushes and invokes the builtin load function (for stack consistency)
  Expr v;

  stack_push(rls, tag_obj(LoadFun));
  mk_str_s(rls, fname);
  v = exec_code(rls, 1, 0);
  return v;
}

Expr exec_code(RlState* rls, int nargs, int flags) {
  #define ARGS rls->base
  #define EXEC rls->exec

  void* labels[] = {
    [OP_NOOP]        = &&op_noop,
    [OP_DUMMY_REF]   = &&op_dummy_ref,

    // stack manipulation -----------------------------------------------------
    [OP_POP]         = &&op_pop,
    [OP_RPOP]        = &&op_rpop,

    // constant loads ---------------------------------------------------------
    [OP_TRUE]        = &&op_true,
    [OP_FALSE]       = &&op_false,
    [OP_NUL]         = &&op_nul,
    [OP_EOS]         = &&op_eos,
    [OP_ZERO]        = &&op_zero,
    [OP_ONE]         = &&op_one,

    // inlined loads ----------------------------------------------------------
    [OP_GLYPH]       = &&op_glyph,
    [OP_SMALL]       = &&op_small,

    // environment instructions -----------------------------------------------
    [OP_GET_VALUE]   = &&op_get_value,
    [OP_GET_GLOBAL]  = &&op_get_global,
    [OP_SET_GLOBAL]  = &&op_set_global,
    [OP_ADD_METHOD]  = &&op_add_method,
    [OP_GET_LOCAL]   = &&op_get_local,
    [OP_SET_LOCAL]   = &&op_set_local,
    [OP_GET_UPVAL]   = &&op_get_upval,
    [OP_SET_UPVAL]   = &&op_set_upval,

    // jump instructions ------------------------------------------------------
    [OP_JUMP]        = &&op_jump,
    [OP_JUMP_F]      = &&op_jump_f,
    [OP_PJUMP_F]     = &&op_pjump_f,
    [OP_PJUMP_T]     = &&op_pjump_t,

    // function call instructions ---------------------------------------------
    [OP_CLOSURE]     = &&op_closure,
    [OP_CAPTURE]     = &&op_capture,
    [OP_CALL]        = &&op_call,
    [OP_RETURN]      = &&op_return,

    // error handling ---------------------------------------------------------
    [OP_CATCH]       = &&op_catch,
    [OP_RAISE]       = &&op_raise,
    [OP_ECATCH]      = &&op_end_catch,

    // arithmetic instructions ------------------------------------------------
    [OP_ADD]         = &&op_add,
    [OP_SUB]         = &&op_sub,
    [OP_MUL]         = &&op_mul,
    [OP_DIV]         = &&op_div,
    [OP_REM]         = &&op_rem,
    [OP_NEQ]         = &&op_neq,
    [OP_NLT]         = &&op_nlt,
    [OP_NGT]         = &&op_ngt,

    // miscellaneous builtins -------------------------------------------------
    [OP_EGAL]        = &&op_egal,
    [OP_HASH]        = &&op_hash,
    [OP_ISA]         = &&op_isa,
    [OP_TYPE]        = &&op_type,

    // list operations --------------------------------------------------------
    [OP_LIST]        = &&op_list,
    [OP_CONS_2]      = &&op_cons_2,
    [OP_CONS_N]      = &&op_cons_n,
    [OP_HEAD]        = &&op_head,
    [OP_TAIL]        = &&op_tail,
    [OP_LIST_REF]    = &&op_list_ref,
    [OP_LIST_LEN]    = &&op_list_len,

    // symbol operations ------------------------------------------------------
    [OP_GENSYM_0]    = &&op_gensym_0,
    [OP_GENSYM_1]    = &&op_gensym_1,

    // string operations ------------------------------------------------------
    [OP_STR]         = &&op_str,
    [OP_CHARS]       = &&op_chars,
    [OP_STR_REF]     = &&op_str_ref,
    [OP_STR_LEN]     = &&op_str_len,

    // tuple operations -------------------------------------------------------
    [OP_TUPLE]       = &&op_tuple,
    [OP_TUPLE_REF]   = &&op_tuple_ref,
    [OP_TUPLE_LEN]   = &&op_tuple_len,

    // interpreter builtins ---------------------------------------------------
    [OP_READ]        = &&op_read,
    [OP_EVAL]        = &&op_eval,
    [OP_PRINT]       = &&op_print,
    [OP_REPL]        = &&op_repl,
    [OP_APPLY]       = &&op_apply,
    [OP_COMPILE]     = &&op_compile,
    [OP_EXEC]        = &&op_exec,
    [OP_LOAD]        = &&op_load,

    // IO helpers -------------------------------------------------------------
    [OP_NEWLINE]     = &&op_newline,

    // environment operations -------------------------------------------------
    [OP_DEFINED]     = &&op_defined,
    [OP_LOCAL_ENV]   = &&op_local_env,
    [OP_GLOBAL_ENV]  = &&op_global_env,

    // system instructions ----------------------------------------------------
    [OP_HEAP_REPORT] = &&op_heap_report,
    [OP_STACK_REPORT]= &&op_stack_report,
    [OP_ENV_REPORT]  = &&op_env_report,
    [OP_STACK_TRACE] = &&op_stack_trace,
    [OP_METHODS]     = &&op_methods,
    [OP_DIS]         = &&op_dis,

    // FFI operations ---------------------------------------------------------
    [OP_FFI_OPEN]    = &&op_ffi_open,
    [OP_FFI_SYM]     = &&op_ffi_sym,
    [OP_FFI_CALL]    = &&op_ffi_call,
    [OP_FFI_CLOSE]   = &&op_ffi_close,
  };
  (void)flags;
  flags = 0;
  // StackRef s_top = rls->s_top;
  FrameRef f_top = rls->f_top;
  StackRef s_base = rls->s_top - nargs;
  int argc=nargs, argx, argy;
    OpCode op;
  Expr x, y;
  Num rx, ry;
  long ix, iy;
  List* lx, * ly;
  Fun* fx;
  Method* method;
  MethodTable* mtx;
  Sym* nx;
  Str* sx;
  Ctl* cx;
  Tuple* tx;
  LibHandle* lhx;
  ForeignFn* ffx;

  // initialize
  goto do_call;

 fetch:
  op = next_op(rls);

  goto *labels[op];

  // miscellaneous instructions -----------------------------------------------
 op_noop:
  goto fetch;

 op_dummy_ref: // should never leak into completed code
  unreachable();

  // stack manipulation instructions ------------------------------------------
 op_pop: // remove TOS
  stack_pop(rls);
  goto fetch;

 op_rpop: // move TOS to TOS-1, then decrement sp
  stack_rpop(rls);
  goto fetch;

  // constant loads -----------------------------------------------------------
 op_true:
  stack_push(rls, TRUE);
  goto fetch;

 op_false:
  stack_push(rls, FALSE);
  goto fetch;

 op_nul:
  stack_push(rls, NUL);
  goto fetch;

 op_eos:
  stack_push(rls, EOS);

 op_zero:
  stack_push(rls, RL_ZERO);
  goto fetch;

 op_one:
  stack_push(rls, RL_ONE);
  goto fetch;

 op_glyph:
  argx = next_op(rls);
  x = tag_glyph(argx);
  stack_push(rls, x);
  goto fetch;

 op_small:
  argx = (short)next_op(rls);
  x = tag_num(argx);
  stack_push(rls, x);
  goto fetch;
  
  // value/variable instructions ----------------------------------------------
 op_get_value: // load a value from the constant store
  argx = next_op(rls);
  x = method_vals(rls->exec)[argx];
  stack_push(rls, x);
  goto fetch;

 op_get_global:
  argx = next_op(rls); // previously resolved index in global environment
  x = toplevel_env_ref(rls, Vm.globals, argx);
  require(rls, x != NONE, "undefined reference.");
  stack_push(rls, x);
  goto fetch;

 op_set_global:
  argx = next_op(rls);
  x = tos(rls);
  toplevel_env_refset(rls, Vm.globals, argx, x);
  goto fetch;

 op_add_method: // used to add methods to generic functions and to initialize closures.
  // leaves the method's function on the stack
  method = as_method(tos(rls));
  fun_add_method(rls, method->fun, method);
  tos(rls) = tag_obj(method->fun);
  goto fetch;

 op_get_local:
  argx = next_op(rls);
  x = ARGS[argx];
  stack_push(rls, x);
  goto fetch;

 op_set_local:
  argx = next_op(rls);
  x = tos(rls);
  ARGS[argx] = x;
  goto fetch;

 op_get_upval:
  argx = next_op(rls);
  x = upval_ref(rls->exec, argx);
  stack_push(rls, x);
  goto fetch;

 op_set_upval:
  argx = next_op(rls);
  x = tos(rls);
  upval_set(rls->exec, argx, x);
  goto fetch;

  // branching instructions ---------------------------------------------------
 op_jump: // unconditional jumping
  argx = next_op(rls);
  rls->pc += argx;
  goto fetch;

 op_jump_f: // jump if TOS is falsey
  argx   = next_op(rls);
  x      = stack_pop(rls);

  if ( is_falsey(x) )
    rls->pc += argx;

  goto fetch;

  // pjump instructions (peek and jump - used to implement and/or)
 op_pjump_f:
  argx = next_op(rls);
  x = tos(rls);

  if ( is_falsey(x) )
    rls->pc += argx;

  else
    stack_pop(rls);

  goto fetch;

 op_pjump_t:
  argx = next_op(rls);

  if ( !is_falsey(x) )
    rls->pc += argx;

  else
    stack_pop(rls);

  goto fetch;

  // closures and function calls ----------------------------------------------
 op_call:
  argc = next_op(rls);

 do_call:
  x = rls->s_top[-argc-1];

  // Dispatch to appropriate method
  fx = as_fun_s(rls, x);
  method = fun_get_method(fx, argc);
  require(rls, method != NULL,
          "%s has no method for %d arguments", fn_name(fx), argc);

  if ( is_user_method(method) )
    goto call_user_method;

  // At present builtin methods save caller state and install themselves in the
  // call frame just like user methods. This helps to make error reporting more
  // consistent
  save_call_frame_s(rls);
  install_method(rls, method);
  op = method->label;
  goto *labels[op];

 call_user_method:
  if ( method_va(method) ) {
    // collect extra args into a list
    int extra = argc - method_argc(method);
    List* rest = mk_list(rls, extra);
    stack_popn(rls, extra);
    stack_push(rls, tag_obj(rest));
    argc = method_argc(method) + 1;
  }

  save_call_frame_s(rls); // save caller state
  install_method(rls, method);

  goto fetch;

  install_method(rls, method);
  op = method->label;
  goto *labels[op];

 op_closure:
  // Get the Fun from TOS, extract its Method, create a closure
  method = as_method(tos(rls));
  method = mk_closure(rls, method);
  tos(rls) = tag_obj(method); // make sure new Fun is visible to GC
  argc = next_op(rls);

  for ( int i=0; i < argc; i++ ) {
    argx = next_op(rls);
    argy = next_op(rls);

    if ( argx == 0 ) // nonlocal, use existing upvalue
      method->upvs[i] = rls->exec->upvs[argy];

    else // local, capture from current scope
      method->upvs[i] = get_upv(rls, rls->base+argy);
  }
  goto fetch;

  // emmitted before a frame with local upvalues returns
 op_capture:
  close_upvs(rls, rls->base);

  goto fetch;

 op_return:
  x = stack_pop(rls); // return value
  restore_call_frame(rls);

  // function that we entered the VM to execute has returned, returned from exec
  if ( rls->f_top == f_top ) {
    if ( rls->f_top == rls->frames ) // special case of first function on the stack
      rls->s_top = s_base-1;
    return x;
  }

  stack_push(rls, x);

  if ( is_builtin_method(rls->exec) )
    goto op_return;

  else
    goto fetch;

  // error handling -----------------------------------------------------------
  op_catch:
  // stack should be closures for handler and body, in that order
  cx = mk_ctl(rls, as_fun(rls->s_top[-2])); // build control object
  rls->s_top[-2] = tag_obj(cx); // replace handler on stack with control object
  save_error_state(rls, 2); // set esc pointer

#ifdef RASCAL_DEBUG
  // stack_report(rls, -1, "stack after installing catch");
#endif

  if ( !rl_setjmp(rls) ) {
    // simple case, set up call and jump to body
    argc = 0;
    goto do_call;
  } else {
  // error was thrown
    rls->pc++; // skip over 'op_end_catch' -- only for normal exit
    x = stack_pop(rls); // error object
    cx = restore_error_state(rls); // restore registers
    rls->esc = cx->esc; // restore enclosing esc pointer
    rls->s_top[-2] = tag_obj(cx->handler); // replace ctl object with handler
    rls->s_top[-1] = x; // set up handler call
    argc = 1;
    goto do_call;
  }

 op_raise:
  rl_longjmp(rls, USER_ERROR);

 op_end_catch:
  x = stack_pop(rls); // return value
  cx = get_error_state(rls); // get current error frame
  rls->s_top = rls->esc; // pop everything pushed by catch
  rls->esc = cx->esc; // restore enclosing escape pointer
  stack_push(rls, x); // push return value back onto stack
  goto fetch;

  // builtin functions --------------------------------------------------------
  // at some hypothetical point in --------------------------------------------
  // the future these will be inlineable --------------------------------------
 op_add:
  rx = as_num_s(rls, ARGS[0]);
  ry = as_num_s(rls, ARGS[1]);
  stack_push(rls, tag_num(rx + ry));

  goto op_return;

 op_sub:
  rx = as_num_s(rls, ARGS[0]);
  ry = as_num_s(rls, ARGS[1]);
  stack_push(rls, tag_num(rx-ry));

  goto op_return;

 op_mul:
  rx = as_num_s(rls, ARGS[0]);
  ry = as_num_s(rls, ARGS[1]);
  stack_push(rls, tag_num(rx * ry));
  goto op_return;

 op_div:
  rx = as_num_s(rls, ARGS[0]);
  ry = as_num_s(rls, ARGS[1]);
  require(rls, ry != 0, "division by zero");
  stack_push(rls, tag_num(rx / ry));
  goto op_return;

 op_rem:
  ix = as_num_s(rls, ARGS[0]);
  iy = as_num_s(rls, ARGS[1]);
  require(rls, ry != 0, "division by zero");
  stack_push(rls, tag_num(ix % iy));
  goto op_return;

 op_neq:
  require_argtype(rls, &NumType, ARGS[0]);
  require_argtype(rls, &NumType, ARGS[1]);
  stack_push(rls, ARGS[0] == ARGS[1] ? TRUE : FALSE);
  goto op_return;

 op_nlt:
  rx = as_num_s(rls, ARGS[0]);
  ry = as_num_s(rls, ARGS[1]);
  stack_push(rls, rx < ry ? TRUE : FALSE);
  goto op_return;

 op_ngt:
  rx = as_num_s(rls, ARGS[0]);
  ry = as_num_s(rls, ARGS[1]);
  stack_push(rls, rx > ry ? TRUE : FALSE);
  goto op_return;

 op_egal:
  x = ARGS[0];
  y = ARGS[1];
  stack_push(rls, egal_exps(x, y) ? TRUE : FALSE);
  goto op_return;

 op_hash:
  ix = hash_exp(ARGS[0]) & XVMSK; // really this should be done consistently elsewhere
  stack_push(rls, tag_num(ix));
  goto op_return;

 op_isa:{
    Type* t;
    x = ARGS[0];
    t = as_type_s(rls, ARGS[1]);
    stack_push(rls, has_type(x, t) ? TRUE : FALSE);
    goto op_return;
  }

 op_type:
  x = ARGS[0];
  y = tag_obj(type_of(x));
  stack_push(rls, y);
  goto op_return;

 op_list:
  lx = mk_list(rls, argc);
  stack_push(rls, tag_obj(lx));
  goto op_return;

 op_cons_2:
#ifdef RASCAL_DEBUG
  // stack_report(rls, argc, "arguments to cons/2 (%d arguments)", argc);
#endif
  lx = as_list_s(rls, ARGS[1]);
  ly = cons(rls, ARGS[0], lx);
  stack_push(rls, tag_obj(ly));
  goto op_return;

 op_cons_n:
#ifdef RASCAL_DEBUG
  // stack_report(rls, argc, "arguments to cons/2+ (%d arguments)", argc);
#endif

  require_argtype(rls, &ListType, tos(rls));
  lx = cons_n(rls, argc);
  stack_push(rls, tag_obj(lx));
  goto op_return;

 op_head:
  x = ARGS[0];
  lx = as_list_s(rls, x);
  require(rls, lx->count > 0, "head", "can't call head on empty list.");
  y  = lx->head;
  tos(rls) = y;
  goto op_return;

 op_tail:
  x = ARGS[0];
  lx = as_list_s(rls, x);
  require(rls, lx->count > 0, 0, "can't call tail on empty list.");
  ly = lx->tail;
  stack_push(rls, tag_obj(ly));
  goto op_return;

 op_list_ref:
  argx = as_num_s(rls, ARGS[1]);
  lx = as_list_s(rls, ARGS[0]);
  require(rls, argx < (int)lx->count, "index out of bounds");
  x = list_ref(lx, argx);
  stack_push(rls, x);
  goto op_return;

 op_list_len:
  lx = as_list_s(rls, ARGS[0]);
  stack_push(rls, tag_num(lx->count));
  goto op_return;

 op_gensym_0:
  mk_gensym_s(rls, "symbol");
  goto op_return;

 op_gensym_1:
  sx = as_str_s(rls, ARGS[0]);
  mk_gensym_s(rls, str_val(sx));
  goto op_return;

 op_str:{ // string constructor (two modes, accepts characters or list of characters)    
    if ( argc == 1 && is_list(ARGS[0]) ) {
      x = stack_pop(rls);
      argc = push_list(rls, as_list(x)); // push list items onto stack (verified in next step)
  }

  // create a buffer to hold the characters
  char buf[argc+1] = {};

  for ( int i=argc-1; i >=0; i-- ) {
    x = stack_pop(rls);
    buf[i] = as_glyph_s(rls, x);
  }

  sx = mk_str_s(rls, buf);
  goto op_return;
  }

 op_chars:
  x = stack_pop(rls);
  sx = as_str_s(rls, x);
  argc = sx->count;

  for ( int i=0; i<argc; i++ ) {
    argx = sx->val[i];
    x = tag_glyph(argx);
    stack_push(rls, x);
  }

  goto op_list;

 op_str_ref:
  argx = as_num_s(rls, ARGS[1]);
  sx = as_str_s(rls, ARGS[0]);
  require(rls, argx < (int)sx->count, "index out of bounds.");
  argy = sx->val[argx];
  x = tag_glyph(argy);
  tos(rls) = x;
  goto op_return;

 op_str_len:
  sx = as_str_s(rls, ARGS[0]);
  stack_push(rls, tag_num(sx->count));
  goto op_return;

 op_tuple:
  mk_tuple_s(rls, argc);
  goto op_return;

 op_tuple_ref:
  argx = as_num_s(rls, ARGS[1]);
  tx = as_tuple_s(rls, ARGS[0]);
  require(rls, argx < (int)tx->count, "index out of bounds.");
  stack_push(rls, tx->data[argx]);
  goto op_return;

 op_tuple_len:
  tx = as_tuple_s(rls, ARGS[0]);
  stack_push(rls, tag_num(tx->count));
  goto op_return;

 op_read:
  require_argtype(rls, &PortType, ARGS[0]);
  x = read_exp(rls, as_port(ARGS[0]), NULL);
  goto op_return;

 op_eval:
  x = eval_exp(rls, ARGS[0]);
  stack_push(rls, x);
  goto op_return;

 op_print:
  require_argtype(rls, &PortType, ARGS[0]);
  print_exp(as_port(ARGS[0]), ARGS[1]);
  stack_push(rls, NUL);
  goto op_return;

 op_repl:
  repl(rls);
  goto op_return; // unreachable at present but seems like good practice

 op_apply:
  fx = as_fun_s(rls, ARGS[0]);
  stack_push(rls, ARGS[0]);
  argx = 0;
  // set up call arguments
  for ( int i=1; i<argc; i++ ) {
    if ( is_list(ARGS[i]) ) {
      argx += push_list(rls, as_list(ARGS[i]));
    } else {
      stack_push(rls, ARGS[i]);
      argx++;
    }
  }

  argc = argx;
  goto do_call;

 op_compile:
  y = ARGS[0]; // expression to compile
  fx = toplevel_compile(rls, y);
  x = fx ? tag_obj(fx) : NUL;
  stack_push(rls, x);
  goto op_return;

 op_exec:
  fx = as_fun_s(rls, ARGS[0]);
  require(rls, is_singleton_fn(fx), "ambiguous call.");
  require(rls, is_user_method(fx->method), "not a user function.");
  // set up call and jump
  stack_push(rls, ARGS[0]);
  argc = 0;
  goto do_call;

 op_load:
  // compile the file and begin executing
  sx = as_str_s(rls, ARGS[0]);
  fx = compile_file(rls, sx->val);

  if ( fx == NULL ) { // compilation failed
    stack_push(rls, NUL);
    goto op_return;
  } else {
    stack_push(rls, tag_obj(fx));
    argc = 0;
    goto do_call;
  }

 op_newline:{
    Port* px = as_port_s(rls, ARGS[0]);
    port_newline(px, 1);
    stack_push(rls, NUL); // dummy return value
    goto op_return;
  }

 op_defined:{
    Sym* n = as_sym_s(rls, ARGS[0]);
    Env* e = as_env_s(rls, ARGS[1]);
    Ref* r = env_resolve(rls, e, n, false);
    stack_push(rls, r == NULL ? FALSE : TRUE);
    goto op_return;
  }

 op_local_env:{
    Env* e = as_env_s(rls, ARGS[0]);
    stack_push(rls, is_global_env(e) ? FALSE : TRUE);
    goto op_return;    
  }

 op_global_env:{
    Env* e = as_env_s(rls, ARGS[0]);
    stack_push(rls, is_global_env(e) ? TRUE : FALSE);
    goto op_return;
  }

 op_heap_report:
  heap_report(rls);
  stack_push(rls, NUL); // dummy return value
  goto op_return;

 op_stack_report:
  stack_report(rls, stack_size(rls), "stack report");
  stack_push(rls, NUL); // dummy return value
  goto op_return;

 op_env_report:{
  env_report(rls, rls->vm->globals);
  stack_push(rls, NUL); // dummy return value
  goto op_return;
  }

 op_stack_trace:
  print_stack_trace(rls);
  stack_push(rls, NUL); // dummy return value
  goto op_return;

 op_methods:
  // return a list of all of a function's methods
  fx = as_fun_s(rls, ARGS[0]);
  argx = 0;

  assert(fx->mcount > 0);

  if ( is_singleton_fn(fx) ) {
    stack_push(rls, tag_obj(fx->method));
    argx = 1;
  } else {
    mtx = fx->methods;

    for ( int i=0; i<mtx->methods.count; i++ ) {
      Method* mx = mtx->methods.data[i];
      stack_push(rls, tag_obj(mx));
      argx++;
    }

    if ( mtx->variadic ) {
      stack_push(rls, tag_obj(mtx->variadic));
      argx++;
    }
  }

  mk_list_s(rls, argx);

  goto op_return;

 op_dis:
  fx = as_fun_s(rls, ARGS[0]);
  ix = as_num_s(rls, ARGS[1]);
  // Disassemble the singleton method (or first method if multimethod)
  method =  fun_get_method(fx, ix);
  require(rls, method != NULL, "no method for %s/%d", fn_name(fx), ix);
  require(rls, is_user_method(method),
          "can't disassemble builtin method for %s/%d", fn_name(fx), ix);
  disassemble_method(method);
  stack_push(rls, NUL); // dummy return value
  goto op_return;

  // FFI operations -----------------------------------------------------------
 op_ffi_open: {
    sx = as_str_s(rls, ARGS[0]);
    char* path = str_val(sx);

    void* handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
    if (handle == NULL) {
      eval_error(rls, "ffi-open failed: %s", dlerror());
    }

    lhx = mk_lib_handle_s(rls, handle, sx);
    goto op_return;
  }

 op_ffi_sym: {
    lhx = as_lib_handle_s(rls, ARGS[0]);
    sx = as_str_s(rls, ARGS[1]);

    require(rls, lhx->handle != NULL, "library handle is closed");

    dlerror(); // Clear any existing error
    void* fn = dlsym(lhx->handle, str_val(sx));
    char* error = dlerror();

    if (error != NULL) {
      eval_error(rls, "ffi-sym failed: %s", error);
    }

    ffx = mk_foreign_fn_s(rls, fn, sx, lhx);
    goto op_return;
  }

 op_ffi_call: {
    // ARGS[0] = ForeignFn
    // ARGS[1] = return type symbol (:int, :double, :ptr, :void, :str)
    // ARGS[2] = arg types list ([:int :str ...])
    // ARGS[3..] = actual arguments

    ffx = as_foreign_fn_s(rls, ARGS[0]);
    Sym* ret_sym = as_sym_s(rls, ARGS[1]);
    List* arg_types_list = as_list_s(rls, ARGS[2]);

    FfiTypeCode ret_type = sym_to_ffi_type(rls, ret_sym);
    int ffi_argc = arg_types_list->count;

    require(rls, argc - 3 == ffi_argc,
            "ffi-call: argument count mismatch, expected %d, got %d",
            ffi_argc, argc - 3);

    // Parse argument types
    FfiTypeCode* arg_types = NULL;
    if (ffi_argc > 0) {
      arg_types = allocate(rls, ffi_argc * sizeof(FfiTypeCode));
      List* tl = arg_types_list;
      for (int i = 0; i < ffi_argc; i++) {
        Sym* tsym = as_sym_s(rls, tl->head);
        arg_types[i] = sym_to_ffi_type(rls, tsym);
        tl = tl->tail;
      }
    }

    // Call
    x = ffi_do_call(rls, ffx, ret_type, ffi_argc, arg_types, &ARGS[3]);

    if (arg_types) release(rls, arg_types, ffi_argc * sizeof(FfiTypeCode));

    stack_push(rls, x);
    goto op_return;
  }

 op_ffi_close: {
    lhx = as_lib_handle_s(rls, ARGS[0]);
    close_lib_handle(rls, lhx);
    stack_push(rls, NUL);
    goto op_return;
  }

  #undef ARGS
  #undef EXEC
}
