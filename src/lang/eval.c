/**
 *
 * Toplevel interpreter API.
 *
 **/
// headers --------------------------------------------------------------------
#include "data/data.h"

#include "sys/sys.h"

#include "lang/eval.h"
#include "lang/read.h"
#include "lang/compile.h"
#include "lang/exec.h"
#include "lang/print.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------
Expr eval_exp(Expr x) {
  Expr v;

  if ( is_literal(x) )
    v = x;

  else if ( is_sym(x) ) {
    Sym* s = as_sym(x);

    if ( is_keyword(s) )
      v = x;

    else {
      v = toplevel_env_get(&Globals, s);
      require(v != NONE, "unbound symbol '%s'", s->val->val);
    }
  } else {
    List* l = as_list(x);

    if ( l->count == 0 ) // empty list is self-evaluating
      v = x;

    else {
      Fun* fun;

      fun = toplevel_compile(l);
      v = exec_code(fun);
    }
  }

  return v;
}

// load -----------------------------------------------------------------------
List* read_file(Port* in) {
  // return a list of all the expressions in a file
  List* out  = NULL;
  int   n    = 0; 
  Expr* base = &Vm.vals[Vm.sp];

  while ( !peof(in) ) {
    Expr x = read_exp(in);

    vpush(x);
    n++;
  }

  out = mk_list(n, base);

  vpopn(n);

  return out;
}

Expr load_file(char* fname) {
  // setup save point
  save_ctx();

  Port* in = NULL;
  Expr out = NUL;

  if ( safepoint() ) {
    recover(NULL);
  } else {
    in          = open_port(fname, "r");
    List* exprs = read_file(in);
    Fun*  code  = compile_file(exprs);
    out         = exec_code(code);
  }

  // clean up port (if necessary)
  if ( in )
    close_port(in);

  // discard save point
  discard_ctx();

  return out;
}

// repl -----------------------------------------------------------------------
void repl(void) {
  Expr x, v;

  // clean up the token buffer and ensure invalid input
  // is cleared from input stream
  void cleanup(void) {
    reset_token();
    pseek(&Ins, SEEK_SET, SEEK_END);
  }

  // create a catch point
  save_ctx();

  for (;;) {
    if ( safepoint() )
      recover(cleanup);

    else {
      fprintf(stdout, PROMPT" ");
      x = read_exp(&Ins);
      v = eval_exp(x);
      fprintf(stdout, "\n>>> ");
      print_exp(&Outs, v);
      fprintf(stdout, "\n\n");
    }
  }

  // discard catch point
  discard_ctx();
}

// initialization -------------------------------------------------------------
