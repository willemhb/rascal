#include <assert.h>

#include "comp.h"

#include "module.h"
#include "sym.h"
#include "list.h"

/* declarations */
/* internal */
size_t comp_expr(module_t program, val_t x);
size_t comp_lit(module_t program, val_t x);
size_t comp_var(module_t program, val_t x);
size_t comp_combo(module_t program, val_t x);

size_t comp_quote(module_t program, val_t f);
size_t comp_do(module_t program, val_t f);
size_t comp_funcall(module_t program, val_t f);

size_t comp_args(module_t program, val_t a);
size_t comp_seq(module_t program, val_t s);

stx_err_t guard_quote_stx(val_t form);
stx_err_t guard_do_stx(val_t form);

#define check_stx(checker, form) assert(checker(form) == no_stx_err)

/* external */
extern bool is_lit(val_t x);

/* globals */
extern val_t QuoteSym, DoSym;

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
  
  else
    return comp_funcall(module, x);
}

size_t comp_funcall(module_t module, val_t x) {
  val_t  head  = as_cons(x)->car;
  val_t  args  = as_cons(x)->cdr;
  size_t nargs = list_len(args);
  
  comp_expr(module, head);
  comp_args(module, args);
  
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

size_t comp_args(module_t module, val_t args) {
  size_t out = 0;

  while (is_cons(args)) {
    out += comp_expr(module, car(args));
    args = cdr(args);
  }

  return out;
}

size_t comp_seq(module_t module, val_t a) {
  size_t out = 0;

  while (is_cons(a)) {
    out += comp_expr(module, car(a));
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

void comp_init(void) {
  QuoteSym = sym("quote");
  DoSym    = sym("do");
}
