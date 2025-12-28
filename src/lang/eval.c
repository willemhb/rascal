#include "lang/eval.h"
#include "lang/read.h"
#include "lang/compile.h"
#include "lang/exec.h"
#include "lang/print.h"
#include "lang/builtin.h"
#include "val.h"
#include "vm.h"

// Globals --------------------------------------------------------------------
// Prototypes -----------------------------------------------------------------
// Helpers
bool is_literal(Expr x);

// Implementations ------------------------------------------------------------
// Helpers
bool is_literal(Expr x) {
  if ( is_sym(x) )
    return is_keyword(as_sym(x));

  if ( is_list(x) )
    return as_list(x)->count == 0;

  return true;
}

// External
Expr eval_exp(RlState* rls, Expr x) {
  Expr v;

  if ( is_literal(x) )
    v = x;

  else if ( is_sym(x) ) {
    Sym* s = as_sym(x);
    v = toplevel_env_get(rls, Vm.globals, s);
    require(rls, v != NONE, "unbound symbol '%s'", sym_val(s));

  } else {
    Fun* fun = toplevel_compile(rls, x);

    if ( fun == NULL ) // compilation failed
      v = NUL;

    else {
      stack_push(rls, tag_obj(fun));
      v = exec_code(rls, 0, 0);
    }
  }

  return v;
}

void toplevel_repl(RlState* rls) {
  // pushes the builtin repl function and invokes the interpreter
  // ensures stack consistency
  stack_push(rls, tag_obj(ReplFun));
  exec_code(rls, 0, 0);
}

void repl(RlState* rls) {
  Expr x, v = NUL;
  Status s;

  // create a catch point
  save_error_state(rls, 0);

  for (;;) {
    fprintf(stdout, PROMPT" ");

    // set safe point for read (so that errors can be handled properly)
    if ( (s=rl_setjmp(rls)) ) {
      if ( s == USER_ERROR ) {
        print_embed(&Errs, "eval error: ", 0, 0, ".\n", 1, tos(rls));
      }

      restore_error_state(rls);
      clear_port(&Ins);
      port_newline(&Outs, 2);
    } else {
      x = read_exp(rls, &Ins, NULL);
    }

    if ( (s=rl_setjmp(rls)) ) {
      if ( s == USER_ERROR ) {
        print_embed(&Errs, "eval error: ", 0, 0, ".\n", 1, tos(rls));
      }
      restore_error_state(rls);
      clear_port(&Ins);
      port_newline(&Outs, 2);
    } else {
      v = eval_exp(rls, x);
      pprintf(&Outs, "\n>>> ");
      print_exp(&Outs, v);
      port_newline(&Outs, 2);
    }
  }

  // discard catch point
  discard_error_state(rls);
}
