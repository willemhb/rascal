#include "lang/exec.h"
#include "lang/compile.h"
#include "val.h"
#include "vm.h"

// Globals --------------------------------------------------------------------
// Prototypes -----------------------------------------------------------------
// Helpers
bool is_falsey(Expr x);

// Implementations ------------------------------------------------------------
// Helpers
bool is_falsey(Expr x) {
  return x == NONE || x == NUL || x == FALSE;
}

// External
Expr load_file(RlState* rls, char* fname) {
  Expr v; Fun* code;
  save_error_state(rls);

  if ( set_safe_point(rls) ) {
    restore_error_state(rls);
    v = NUL;
  } else {
    code = compile_file(rls, fname);
    push(rls, tag_obj(code));
    v = exec_code(rls, 0, 0);
  }

  discard_error_state(rls);

  return v;
}


Expr exec_code(RlState* rls, int nargs, int flags) {
  void* labels[] = {
    [OP_NOOP]        = &&op_noop,

    // stack manipulation -----------------------------------------------------
    [OP_POP]         = &&op_pop,
    [OP_RPOP]        = &&op_rpop,

    // constant loads ---------------------------------------------------------
    [OP_TRUE]        = &&op_true,
    [OP_FALSE]       = &&op_false,
    [OP_NUL]         = &&op_nul,
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
    [OP_CONS]        = &&op_cons,
    [OP_CONSN]       = &&op_consn,
    [OP_HEAD]        = &&op_head,
    [OP_TAIL]        = &&op_tail,
    [OP_LIST_REF]    = &&op_list_ref,
    [OP_LIST_LEN]    = &&op_list_len,

    // string operations ------------------------------------------------------
    [OP_STR]         = &&op_str,
    [OP_CHARS]       = &&op_chars,
    [OP_STR_REF]     = &&op_str_ref,
    [OP_STR_LEN]     = &&op_str_len,

    // interpreter builtins ---------------------------------------------------
    [OP_APPLY]       = &&op_apply,
    [OP_COMPILE]     = &&op_compile,
    [OP_EXEC]        = &&op_exec,
    [OP_LOAD]        = &&op_load,
    [OP_ERROR]       = &&op_error,

    // environment operations -------------------------------------------------
    [OP_DEFINED]     = &&op_defined,
    [OP_LOCAL_ENV]   = &&op_local_env,
    [OP_GLOBAL_ENV]  = &&op_global_env,

    // system instructions ----------------------------------------------------
    [OP_HEAP_REPORT] = &&op_heap_report,
    [OP_STACK_REPORT]= &&op_stack_report,
    [OP_ENV_REPORT]  = &&op_env_report,
    [OP_DIS]         = &&op_dis,
  };

  (void)flags; // not used yet
  int argc=nargs, argx, argy;
  OpCode op;
  Expr x, y, z;
  Num nx, ny, nz;
  long ix, iy, iz;
  List* lx, * ly;
  Fun* fx;
  Method* method;
  Str* sx;

  // initialize
  goto do_call;

 fetch:
  op = next_op(rls);

  goto *labels[op];

  // miscellaneous instructions -----------------------------------------------
 op_noop:
  goto fetch;

  // stack manipulation instructions ------------------------------------------
 op_pop: // remove TOS
  pop(rls);
  goto fetch;

 op_rpop: // move TOS to TOS-1, then decrement sp
  rpop(rls);
  goto fetch;

  // constant loads -----------------------------------------------------------
 op_true:
  push(rls, TRUE);
  goto fetch;

 op_false:
  push(rls, FALSE);
  goto fetch;

 op_nul:
  push(rls, NUL);
  goto fetch;

 op_zero:
  push(rls, RL_ZERO);
  goto fetch;

 op_one:
  push(rls, RL_ONE);
  goto fetch;

 op_glyph:
  argx = next_op(rls);
  x = tag_glyph(argx);
  push(rls, x);
  goto fetch;

 op_small:
  argx = (short)next_op(rls);
  x = tag_num(argx);
  push(rls, x);
  goto fetch;
  
  // value/variable instructions ----------------------------------------------
 op_get_value: // load a value from the constant store
  argx = next_op(rls);
  x = alist_get(rls->fn->chunk->vals, argx);
  push(rls, x);
  goto fetch;

 op_get_global:
  argx = next_op(rls); // previously resolved index in global environment
  x = toplevel_env_ref(rls, Vm.globals, argx);
  require(rls, x != NONE, "undefined reference");
  push(rls, x);
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
  x = rls->stack[rls->bp+argx];
  push(rls, x);
  goto fetch;

 op_set_local:
  argx = next_op(rls);
  x = tos(rls);
  rls->stack[rls->bp+argx] = x;
  goto fetch;

 op_get_upval:
  argx = next_op(rls);
  x = upval_ref(rls->fn, argx);
  push(rls, x);
  goto fetch;

 op_set_upval:
  argx = next_op(rls);
  x = tos(rls);
  upval_set(rls->fn, argx, x);
  goto fetch;

  // branching instructions ---------------------------------------------------
 op_jump: // unconditional jumping
  argx = next_op(rls);
  rls->pc += argx;
  goto fetch;

 op_jump_f: // jump if TOS is falsey
  argx   = next_op(rls);
  x      = pop(rls);

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
    pop(rls);

  goto fetch;

 op_pjump_t:
  argx = next_op(rls);

  if ( !is_falsey(x) )
    rls->pc += argx;

  else
    pop(rls);

  goto fetch;

  // closures and function calls ----------------------------------------------
 op_call:
  argc = next_op(rls);

 do_call:
  x = *stack_ref(rls, -argc-1);

  // Get the Fun and look up the appropriate Method
  fx = as_fun_s(rls, current_fn_name(rls), x);
  method = fun_get_method(fx, argc);
  require(rls, method != NULL, "%s has no method for %d arguments", fn_name(fx), argc);

  if ( is_user_method(method) )
    goto call_user_method;

  if ( method_va(method) )
    require_vargco(rls, method_name(method), method_argc(method), argc);

  else
    require_argco(rls, method_name(method), method_argc(method), argc);

  op = method->label;

  goto *labels[op];

 call_user_method:
#ifdef RASCAL_DEBUG
   if ( !method->fun->macro )
    stack_report_slice(rls, argc+1, "calling user method %s", method_name(method));
#endif

  if ( method_va(method) ) {
    require_vargco(rls, method_name(method), method_argc(method), argc);
    // collect extra args into a list
    int extra = argc - method_argc(method);
    List* rest = mk_list(rls, extra, &rls->stack[rls->sp - extra]);
    popn(rls, extra);
    push(rls, tag_obj(rest));
    argc = method_argc(method) + 1;
  } else {
    require_argco(rls, method_name(method), method_argc(method), argc);
  }

  save_call_frame(rls, argc); // save caller state
  install_method(rls, method, argc);

  goto fetch;

 op_closure:
  // Get the Fun from TOS, extract its Method, create a closure
  method = as_method(tos(rls));
  method = mk_closure(rls, method);
  tos(rls) = tag_obj(method); // make sure new Fun is visible to GC
  argc = next_op(rls);

  for ( int i=0; i < argc; i++ ) {
    argx = next_op(rls);
    argy = next_op(rls);

    if ( argx == 0 ) // nonlocal
      method->upvs.vals[i] = rls->fn->upvs.vals[argy];

    else
      method->upvs.vals[i] = get_upv(rls, rls->stack+rls->bp+argy);
  }
  goto fetch;

  // emmitted before a frame with local upvalues returns
 op_capture:
  close_upvs(rls, rls->stack+rls->bp);
  goto fetch;

 op_return:{
#ifdef RASCAL_DEBUG
    char* returning = current_fn_name(rls);
    stack_report_slice(rls, (rls->sp+1)-rls->bp, "before returning from %s", returning);
#endif

    x = pop(rls);

    if ( restore_call_frame(rls) ) { // more work to do
      push(rls, x);

#ifdef RASCAL_DEBUG
      stack_report_slice(rls, (rls->sp+1)-rls->bp, "after returning from %s", returning);
#endif
      goto fetch;
    } else { // reached toplevel, exit
      popn(rls, nargs+1); // remove caller and original arguments from stack
      return x;
    }
  }

  // builtin functions --------------------------------------------------------
  // at some hypothetical point in --------------------------------------------
  // the future these will be inlineable --------------------------------------
 op_add:
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, "+", x);
  ny = as_num_s(rls, "+", y);
  nz = nx + ny;
  z = tag_num(nz);
  tos(rls) = z;           // combine push/pop
  goto fetch;

 op_sub:
  y      = pop(rls);
  x      = pop(rls);
  nx     = as_num_s(rls, "-", x);
  ny     = as_num_s(rls, "-", y);
  nz     = nx - ny;
  z      = tag_num(nz);
  tos(rls)  = z;           // combine push/pop

  goto fetch;

 op_mul:
  y      = pop(rls);
  x      = pop(rls);
  nx     = as_num_s(rls, "*", x);
  ny     = as_num_s(rls, "*", y);
  nz     = nx * ny;
  z      = tag_num(nz);
  tos(rls)  = z;           // combine push/pop
  goto fetch;

 op_div:
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, "/", x);
  ny = as_num_s(rls, "/", y); require(rls, ny != 0, "division by zero");
  nz = nx / ny;
  z  = tag_num(nz);
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_rem:
  y = pop(rls);
  x = pop(rls);
  ix = as_num_s(rls, "rem", x);
  iy = as_num_s(rls, "rem", y); require(rls, ny != 0, "division by zero");
  iz = ix % iy;
  z  = tag_num(iz);
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_neq:
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, "=", x);
  ny = as_num_s(rls, "=", y);
  z = nx == ny ? TRUE : FALSE;
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_nlt:
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, "<", x);
  ny = as_num_s(rls, "<", y);
  z = nx < ny ? TRUE : FALSE;
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_ngt:
  y = pop(rls);
  x = pop(rls);
  nx = as_num_s(rls, ">", x);
  ny = as_num_s(rls, ">", y);
  z = nx > ny ? TRUE : FALSE;
  tos(rls) = z; // combine push/pop
  goto fetch;

 op_egal:
  y = pop(rls);
  x = pop(rls);
  z = egal_exps(x, y) ? TRUE : FALSE;
  tos(rls) = z;
  goto fetch;

 op_hash:
  x = pop(rls);
  ix = hash_exp(x) & XVMSK; // really this should be done consistently elsewhere
  y = tag_num(ix);
  tos(rls) = y;
  goto fetch;

 op_isa:{
    Type* t;
    y = pop(rls);
    x = pop(rls);
    t = as_type_s(rls, "isa?", y);
    z = has_type(x, t) ? TRUE : FALSE;
    tos(rls) = z;
    goto fetch;
  }

 op_type:
  x = pop(rls);
  y = tag_obj(type_of(x));
  tos(rls) = y;
  goto fetch;

 op_list:
  lx = mk_list(rls, argc, &rls->stack[rls->sp-argc]);
  popn(rls, argc);
  tos(rls) = tag_obj(lx);
  goto fetch;

 op_cons:
  lx = as_list_s(rls, "cons", rls->stack[rls->sp-1]);
  ly = cons(rls, rls->stack[rls->sp-2], lx);
  z  = tag_obj(ly);
  popn(rls, 2);
  tos(rls) = z;
  goto fetch;

 op_consn:
  require_argtype(rls, "cons*", &ListType, tos(rls));
  lx = cons_n(rls, argc);
  popn(rls, argc);
  tos(rls) = tag_obj(lx);
  goto fetch;

 op_head:
#ifdef RASCAL_DEBUG
  // printf("\nargument to head: ");
  // print_exp(&Outs, tos(rls));
  // printf("\n");
#endif
  x = pop(rls);
  lx = as_list_s(rls, "head", x);
  require(rls, lx->count > 0, "can't call head on empty list");
  y  = lx->head;
  tos(rls) = y;
  goto fetch;

 op_tail:

#ifdef RASCAL_DEBUG
  // stack_report_slice(rls, 2, "DEBUG - arguments to tail");
#endif

  x = pop(rls);
  lx = as_list_s(rls, "tail", x);
  require(rls, lx->count > 0, "can't call tail on empty list");
  ly = lx->tail;
  tos(rls) = tag_obj(ly);
  goto fetch;

 op_list_ref:
  x = pop(rls);
  argx = as_num_s(rls, "list-ref", x);
  x = pop(rls);
  lx = as_list_s(rls, "list-ref", x);
  require(rls, argx < (int)lx->count, "index out of bounds");
  x = list_ref(lx, argx);
  tos(rls) = x;
  goto fetch;

 op_list_len:
  x = pop(rls);
  lx = as_list_s(rls, "list-len", x);
  tos(rls) = tag_num(lx->count);
  goto fetch;

 op_str:{ // string constructor (two modes, accepts characters or list of characters)    
  if ( argc == 1 && is_list(tos(rls)) ) {
    x = pop(rls);
    lx = as_list(x);
    argc = lx->count;

    // unpack onto stack (type verified later)
    while ( lx->count > 0 ) {
      push(rls, lx->head);
      lx = lx->tail;
    }
  }

  // create a buffer to hold the characters
  char buf[argc+1] = {};

  for ( int i=argc-1; i >=0; i-- ) {
    x = pop(rls);
    buf[i] = as_glyph_s(rls, "str", x);
  }
  
  sx = mk_str(rls, buf);
  tos(rls) = tag_obj(sx);
  goto fetch;
  }

 op_chars:
  x = pop(rls);
  sx = as_str_s(rls, "chars", x);
  argc = sx->count;

  for ( int i=0; i<argc; i++ ) {
    argx = sx->val[i];
    x = tag_glyph(argx);
    push(rls, x);
  }

  goto op_list;

 op_str_ref:
  x = pop(rls);
  argx = as_num_s(rls, "str-ref", x);
  y = pop(rls);
  sx = as_str_s(rls, "str-ref", y);
  require(rls, argx < (int)sx->count, "index out of bounds");
  argy = sx->val[argx];
  x = tag_glyph(argy);
  tos(rls) = x;
  goto fetch;

 op_str_len:
  x = pop(rls);
  sx = as_str_s(rls, "str-len", x);
  tos(rls) = tag_num(sx->count);
  goto fetch;

 op_apply:
  y = pop(rls); // arguments
  x = rpop(rls); // function to apply (removes apply from the stuck)
  ly = as_list_s(rls, "apply", y);
  require_argtype(rls, "apply", &FunType, x);
  argc = push_list(rls, ly);
  goto do_call; // jump directly to do_call (all the right variables are and stack state is correct)

 op_compile:
  y = pop(rls); // expression to compile
  fx = toplevel_compile(rls, y);
  tos(rls) = tag_obj(fx);
  goto fetch;

 op_exec:
  require_argtype(rls, "exec", &FunType, tos(rls));
  rpop(rls); // remove call to exec, leaving the function to be executed on top of the stack
  argc = 0;
  goto do_call;

 op_load:
  // compile the file and begin executing
  x = tos(rls); // needs to be preserved through GC
  sx = as_str_s(rls, "load", x);
  fx = compile_file(rls, sx->val);
  pop(rls); // remove file name from stack
  tos(rls) = tag_obj(fx); // replace `load` call with compiled code
  save_call_frame(rls, 0);
  install_method(rls, fx->method, 0);
  goto fetch;

 op_error:
  x = pop(rls);
  sx = as_str_s(rls, "error", x);
  user_error(rls, sx->val);

 op_defined:{
    y = pop(rls);
    x = pop(rls);
    Sym* n = as_sym_s(rls, "defined?", x);
    Env* e = as_env_s(rls, "defined?", y);
    Ref* r = env_resolve(rls, e, n, false);
    x = r == NULL ? FALSE : TRUE;
    tos(rls) = x;
    goto fetch;
  }

 op_local_env:{
    x = pop(rls);
    Env* e = as_env_s(rls, "local-env?", x);
    x = is_global_env(e) ? FALSE : TRUE;
    tos(rls) = x;
    goto fetch;    
  }

 op_global_env:{
    x = pop(rls);
    Env* e = as_env_s(rls, "global-env?", x);
    x = is_global_env(e) ? TRUE : FALSE;
    tos(rls) = x;
    goto fetch;
  }

 op_heap_report:
  heap_report(rls);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_stack_report:
  stack_report(rls);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_env_report:
  env_report(rls);
  tos(rls) = NUL; // dummy return value
  goto fetch;

 op_dis:
  x = pop(rls);
  fx = as_fun_s(rls, "*dis*", x);
  // Disassemble the singleton method (or first method if multimethod)
  method =  is_singleton_fn(fx) ? fx->methods : fx->methods->methods.vals[0];
  disassemble_method(method);
  tos(rls) = NUL; // dummy return value
  goto fetch;
}
