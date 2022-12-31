#include <assert.h>

#include "comp.h"

#include "module.h"
#include "sym.h"
#include "list.h"
#include "small.h"

#include "error.h"

/* declarations */
/* internal */
size_t comp_expr(module_t program, val_t x);
size_t comp_lit(module_t program, val_t x);
size_t comp_var(module_t program, val_t x);
size_t comp_combo(module_t program, val_t x);

size_t comp_quote(module_t program, val_t f);
size_t comp_do(module_t program, val_t f);
size_t comp_catch(module_t program, val_t f);
size_t comp_def(module_t program, val_t f);
size_t comp_put(module_t program, val_t f);
size_t comp_funcall(module_t program, val_t f);

size_t comp_args(module_t program, val_t a);
size_t comp_seq(module_t program, val_t s);

stx_err_t guard_quote_stx(val_t form);
stx_err_t guard_do_stx(val_t form);
stx_err_t guard_catch_stx(val_t form);
stx_err_t guard_def_stx(val_t form);
stx_err_t guard_put_stx(val_t form);

#define check_stx(checker, form)                               \
  do {                                                         \
    stx_err_t __err__ = checker(form);                         \
    if (__err__ != no_stx_err)                                 \
      panic(0, "syntax error: %s.\n", SyntaxErrors[__err__]);  \
  } while (false)

/* external */
extern bool is_lit(val_t x);

/* globals */
extern val_t QuoteSym, DoSym, CatchSym, PutSym, DefSym, IfSym, FunSym;

char *SyntaxErrors[] = {
  "none", "malformed expression", "illegal expression"
};

/* API */
/* external */
module_t comp(val_t x) {
  module_t out = make_module("<toplevel>", 0, false, NULL, NULL, NULL, NULL);

  comp_expr(out, x);
  emit_instr(out, op_halt);

  return out;
}

/* internal */
size_t comp_expr(module_t program, val_t x) {
  if (is_lit(x))
    return comp_lit(program, x);

  else if (is_sym(x))
    return comp_var(program, x);

  else
    return comp_combo(program, x);
}

size_t comp_lit(module_t module, val_t x) {
  if (x == NUL)
    return emit_instr(module, op_load_nul);

  else if (x == ZERO)
    return emit_instr(module, op_load_small_zero);

  else if (x == ONE)
    return emit_instr(module, op_load_small_one);

  else if (is_small(x) && as_small(x) <= INT16_MAX && as_small(x) >= INT16_MIN) {
    short x16 = as_small(x);

    return emit_instr(module, op_load_small_16, x16);
  }

  size_t n = put_module_const(module, x);

  return emit_instr(module, op_load_const, n);
}

size_t comp_var(module_t module, val_t x) {
  size_t n = put_module_const(module, x);

  return emit_instr(module, op_load_global, n);
}

size_t comp_combo(module_t module, val_t x) {
  val_t head = as_cons(x)->car;

  if (head == QuoteSym)
    return comp_quote(module, x);
  
  else if (head == DoSym)
    return comp_do(module, x);

  else if (head == CatchSym)
    return comp_catch(module, x);

  else if (head == DefSym)
    return comp_def(module, x);

  else if (head == PutSym)
    return comp_put(module, x);

  else
    return comp_funcall(module, x);
}

size_t comp_funcall(module_t module, val_t x) {
  val_t  head  = as_cons(x)->car;
  val_t  args  = as_cons(x)->cdr;
  size_t nargs = list_len(args);
  
  comp_expr(module, head); repanic(0);
  comp_args(module, args); repanic(0);

  return emit_instr(module, op_invoke, nargs);
}

size_t comp_quote(module_t module, val_t form) {
  check_stx(guard_quote_stx, form);

  return comp_lit(module, cadr(form));
}

size_t comp_do(module_t module, val_t form) {
  check_stx(guard_do_stx, form);

  return comp_seq(module, cdr(form));
}

size_t comp_catch(module_t module, val_t form) {
  check_stx(guard_catch_stx, form);

  val_t recover_expr = cadr(form);
  val_t try_exprs = cddr(form);

  size_t fill_1     = emit_instr(module, op_save_prompt);
  size_t jump_1     = emit_instr(module, op_jump, 0);
  size_t expr_1     = comp_expr(module, recover_expr); repanic(0);
  
  size_t fill_2     = expr_1;
  size_t jump_2     = emit_instr(module, op_jump, 0);
  
  comp_seq(module, try_exprs); repanic(0);

  size_t output     = emit_instr(module, op_discard_prompt);
  size_t jump1_size = jump_2 - jump_1;
  size_t jump2_size = output - jump_2;

  fill_input(module, fill_1, jump1_size);
  fill_input(module, fill_2, jump2_size);

  return output;
}

size_t comp_def(module_t module, val_t form) {
  check_stx(guard_def_stx, form);

  val_t name = cadr(form);
  val_t bind = caddr(form);

  define(as_sym(name), NUL);
}

size_t comp_put(module_t module, val_t form) {
  check_stx(guard_put_stx, form);

  val_t name = cadr(form);
  val_t bind = caddr(form);

  
}

size_t comp_args(module_t module, val_t args) {
  size_t out = 0;

  while (is_cons(args)) {
    out += comp_expr(module, car(args)); repanic(0);
    args = cdr(args);
  }

  return out;
}

size_t comp_seq(module_t module, val_t a) {
  size_t out = 0;

  while (is_cons(a)) {
    out += comp_expr(module, car(a)); repanic(0);
    a    = cdr(a);

    if (is_cons(a))
      out += emit_instr(module, op_pop);
  }

  return out;
}

stx_err_t guard_quote_stx(val_t form) {
  if (list_len(form) != 2)
    return malformed_stx_err;

  return no_stx_err;
}

stx_err_t guard_do_stx(val_t form) {
  if (list_len(form) < 2)
    return malformed_stx_err;

  return no_stx_err;
}

stx_err_t guard_catch_stx(val_t form) {
  if (list_len(form) < 3)
    return malformed_stx_err;

  return no_stx_err;
}

stx_err_t guard_def_stx(val_t form) {
  if (list_len(form) != 3)
    return malformed_stx_err;

  if (!is_sym(cadr(form)))
    return illegal_stx_err;

  return no_stx_err;
}

stx_err_t guard_put_stx(val_t form) {
  if (list_len(form) != 3)
    return malformed_stx_err;

  if (!is_sym(cadr(form)))
    return illegal_stx_err;

  if (!is_bound(cadr(form)))
    return illegal_stx_err;

  return no_stx_err;
}

void comp_init(void) {
  QuoteSym = sym("quote");
  DoSym    = sym("do");
  CatchSym = sym("catch");
  PutSym   = sym("put");
  DefSym   = sym("def");
  IfSym    = sym("if");
  FunSym   = sym("fun");
}
