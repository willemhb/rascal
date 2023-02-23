#include "eval.h"
#include "object.h"
#include "runtime.h"
#include "compile.h"
#include "read.h"
#include "print.h"

#include "util/io.h"

// globals --------------------------------------------------------------------
#define PROMPT ">>"

// helpers --------------------------------------------------------------------
extern bool is_literal(Val x);
extern uint op_argc(OpCode opcode);
extern bool is_closed(Frame* frame);
extern void capture_frame(Frame* frame);

bool is_defined(Val x) {
  return has_type(x, SYM) && as_sym(x)->bind != UNDEFINED;
}

bool is_bound(Val x) {
  return has_type(x, SYM) && as_sym(x)->bind != UNBOUND;
}

Val exec_at(Func* closure, OpCode op, uint16 argx, uint16 argy);

Val toplevel(Val n) {
  Val b = as_sym(n)->bind;

  GUARD(b != UNDEFINED,
        NUL,
        EVAL_ERROR,
        "undefined symbol '%s'",
        as_sym(n)->name );

  GUARD(b != UNBOUND,
        NUL,
        EVAL_ERROR,
        "unbound symbol '%s'",
        as_sym(n)->name );

  return b;
}

Val lookup(Val n) {
  if (Interpreter.fp > Interpreter.fb) {
    List* lenv = LENVRX,* cenv = CENVRX;
    Val* slots = SLOTSRX;

    while (lenv != &EmptyList) {
      Vec* formals = as_vec(lenv->head);

      for (uint i=0; i<formals->count; i++) {
        if (formals->array[i] == n)
          return slots[i];
      }

      lenv = lenv->tail;
      cenv = cenv->tail;

      if (cenv != &EmptyList)
        slots = as_vec(cenv->head)->array;
    }
  }

  return toplevel(n);
}

// API ------------------------------------------------------------------------
Val eval(Val x) {
  if (is_literal(x))
    return x;

  if (has_type(x, SYM))
    return lookup(x);

  Func* c = compile(x);

  REPANIC(NUL);

  x = exec(c);

  REPANIC(NUL);

  return x;
}

void repl(void) {
  for (;;) {
    printf(PROMPT" ");
    
    Val x = read();

    if (recover())
      continue;

    Val v = eval(x);

    if (recover())
      continue;

    print(v);
    newln();
  }
}

Val exec(Func* closure) {
  return exec_at(closure, OP_BEGIN, 0, 0);
}

#undef GUARD
#undef REPANIC

#define GUARD(test, sentinel, error, fmt, ...)              \
  do {                                                      \
    if(!(test)) {                                           \
      panic(error, fmt __VA_OPT__(,) __VA_ARGS__);          \
      push(sentinel);                                       \
      goto halt;                                            \
    }                                                       \
  } while (false)

#define REPANIC(sentinel)			\
  do {                              \
    if (panicking()) {				\
      push(sentinel);               \
      goto halt;                    \
    }                               \
  } while (false)

Val exec_at(Func* closure, OpCode op, uint16 argx, uint16 argy) {
  static void* labels[] = {
    [OP_BEGIN]            = &&op_begin,

    [OP_DUP]              = &&op_dup,              [OP_POP]             = &&op_pop,

    [OP_LOAD_NUL]         = &&op_load_nul,

    [OP_LOAD_CONST]       = &&op_load_const,       [OP_LOAD_GLOBAL]     = &&op_load_global,
    [OP_LOAD_LOCAL]       = &&op_load_local,       [OP_LOAD_CLOSURE]    = &&op_load_closure,
 
    [OP_DEF_GLOBAL]       = &&op_def_global,       [OP_DEF_LOCAL]       = &&op_def_local,

    [OP_SET_GLOBAL]       = &&op_set_global,       [OP_SET_LOCAL]       = &&op_set_local,
    [OP_SET_CLOSURE]      = &&op_set_closure,

    [OP_JUMP]             = &&op_jump,             [OP_JUMP_IF_FALSE]   = &&op_jump_if_false,

    [OP_SET_PROMPT]       = &&op_set_prompt,

    [OP_CAPTURE_CLOSURE]  = &&op_capture_closure,  [OP_INVOKE_FUNC]     = &&op_invoke_func,
    [OP_TAIL_INVOKE_FUNC] = &&op_tail_invoke_func, [OP_RETURN]          = &&op_return
  };

  Val x, v;

  int argc;

  List* env; Frame* frame;

  goto *labels[op];

 fetch:
  op = *(IPRX++);

  argc = op_argc(op);

  if (argc)
    argx = *(IPRX++);

  if (argc > 1)
    argy = *(IPRX++);

  goto *labels[op];

 halt:
  recover();
  v = pop();

  goto end;

 end:
  reset_sp();
  reset_fp();

  return v;

 op_begin:
  pushf(closure, argx);

  goto fetch;

 op_dup:
  push(TOSRX);

  goto fetch;

 op_pop:
  x = pop();

  goto fetch;

 op_load_nul:
  push(NUL);

  goto fetch;

 op_load_const:
  push(VALSRX[argx]);

  goto fetch;

 op_load_global:
  x = VALSRX[argx];

  GUARD(is_defined(x),
        NUL,
        EVAL_ERROR,
        "undefined symbol '%s'",
        as_sym(x)->name );

  GUARD(is_bound(x),
        NUL,
        EVAL_ERROR,
        "undefined symbol '%s'",
        as_sym(x)->name );

  push(as_sym(x)->bind);

  goto fetch;

 op_load_local:
  push(SLOTSRX[argx]);

  goto fetch;

 op_load_closure:
  env = CENVRX;
  
  for (uint i=argx; i; i--)
    env = env->tail;

  push(as_vec(env->head)->array[argy]);

  goto fetch;

 op_def_global:
  x = VALSRX[argx];

  if (as_sym(x)->bind == UNDEFINED)
    as_sym(x)->bind = UNBOUND;

  goto fetch;

 op_def_local:
  if (SLOTSRX[argx] == UNDEFINED)
    SLOTSRX[argx] = UNBOUND;

 op_set_global:
  x = VALSRX[argx];

  GUARD(is_defined(x),
        NUL,
        EVAL_ERROR,
        "can't set unbound symbol '%s'",
        as_sym(x)->name );

  as_sym(x)->bind = TOSRX;

  goto fetch;

 op_set_local:
  GUARD(SLOTSRX[argx] != UNDEFINED,
        NUL,
        EVAL_ERROR,
        "can't set unbound symbol '%s'",
        as_sym(x)->name );

  as_sym(x)->bind = TOSRX;

  goto fetch;

 op_set_closure:
  env = CENVRX;

  for (uint i=0; i > argx; i--)
    env = env->tail;

  as_vec(env->head)->array[argx] = TOSRX;

  goto fetch;

 op_jump:
  IPRX += argx;

  goto fetch;

 op_jump_if_false:
  x = pop();

  if (x == NUL)
    IPRX += argx;

  goto fetch;

 op_set_prompt:
  PROMPTRX = &FRAMERX;

  goto fetch;

 op_capture_closure:
  frame = &FRAMERX;
  x     = TOSRX;

  capture_frame(frame);
  
  
  goto fetch;

 op_tail_invoke_func:
 op_invoke_func:
 op_return:
  v = popf();

  if (FPRX == FERX) // last frame returned, exit and return v
    goto end;

  goto fetch;
}
