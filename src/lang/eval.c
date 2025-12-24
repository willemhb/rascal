#include "lang/eval.h"
#include "lang/read.h"
#include "lang/compile.h"
#include "lang/exec.h"
#include "lang/print.h"
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
    push(rls, tag_obj(fun));
    v = exec_code(rls, 0, 0);
  }

  return v;
}

void repl(RlState* rls) {
  Expr x, v = NUL;

  // create a catch point
  save_error_state(rls);

  for (;;) {
    fprintf(stdout, PROMPT" ");

    // set safe point for read (so that errors can be handled properly)
    if ( set_safe_point(rls) ) {
      restore_error_state(rls);
      clear_input(&Ins);
      goto next_line;
    } else {
      x = read_exp(rls, &Ins);
    }

    if ( set_safe_point(rls) ) {
      restore_error_state(rls);
      goto next_line;
    } else {
      v = eval_exp(rls, x);
      pprintf(&Outs, "\n>>> ");
      print_exp(&Outs, v);
    }

  next_line:
    pprintf(&Outs, "\n\n");
  }

  // discard catch point
  discard_error_state(rls);
}
