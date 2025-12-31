#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vm/vm.h"
#include "vm/error.h"
#include "vm/memory.h"
#include "util/util.h"
#include "val.h"
#include "lang.h"

// globals
char* ErrorNames[NUM_ERRORS] = {
  [USER_ERROR]    = "eval",
  [EVAL_ERROR]    = "eval",
  [RUNTIME_ERROR] = "runtime",
  [SYSTEM_ERROR]  = "system"
};

Sym* ErrorSyms[NUM_ERRORS] = {};

char ErrorMessageBuffer[BUFFER_SIZE];

// local helpers
static void print_error_header(Port* ios, Status etype) {
  pprintf(ios, "%s error: ", error_name(etype));
}

static void print_syntax_error_header(Port* ios, List* form) {
  pprintf(ios, "syntax error in %s: ", sym_val(as_sym(form->head)));
}

static void print_type_error(Port* ios, Type* e, Expr g) {
  pprintf(ios, "expected a %s, got ", type_name(e));
  print_expr(ios, g);
  pprintf(ios, ".");
}

static void print_arity_error(Port* ios, char* tpl, bool va, int e, int g) {
  char* fmt;

  if ( va )
    fmt = "expected at least %d %s, got %d.";

  else
    fmt = "expected %d arguments %s, got %d.";

  pprintf(ios, fmt, e, tpl, g);
}

static Str* get_error_message(RlState* rls, Port* tmp) {
  char* msg; Str* out;

  prewind(tmp);
  msg = pgets(ErrorMessageBuffer, BUFFER_SIZE, tmp);
  close_port(tmp);
  out = new_str(rls, msg, 0, false);
  return out;
}

// error state handlers
Ctl* save_error_state(RlState* rls, int offset) {
  Ctl* out;
  if ( offset == 0 ) { // for builtin error handlers
    out = mk_ctl_s(rls, NULL);
    offset = 1;
  } else {
    out = as_ctl(rls->s_top[-offset]);
  }

  init_ctl(rls, out);
  rls->esc = rls->s_top-offset;

#ifdef RASCAL_DEBUG
  // stack_report(rls, -1, "stack on installing error state");
#endif

  return out;
}

Ctl* get_error_state(RlState* rls) {
  if ( rls->esc == NULL )
    return NULL;

  return as_ctl(*rls->esc);
}

Ctl* restore_error_state(RlState* rls) {
  Ctl* ctl = get_error_state(rls);

  assert(ctl != NULL);

  // restore registers (except for esc)
  rls->exec  = ctl->exec;
  rls->pc    = ctl->pc;
  rls->s_top  = ctl->s_top;
  rls->base  = ctl->base;
  rls->f_top = ctl->f_top;

  // return ctl object for further handling
  return ctl;
}

Ctl* discard_error_state(RlState* rls) {
#ifdef RASCAL_DEBUG
  // stack_report(rls, -1, "stack before discarding error state");
#endif

  assert(rls->esc == rls->s_top-1);
  Ctl* ctl = get_error_state(rls);
  assert(ctl != NULL);

  rls->s_top = rls->esc; // pop the error frame
  rls->esc = ctl->esc; // reset esc pointer

#ifdef RASCAL_DEBUG
  // stack_report(rls, -1, "stack after discarding error state");
#endif
  return ctl; // return discarded control object for further processing
}

void rascal_error(RlState* rls, Status etype, bool fatal, Str* message) {
  if ( fatal ) {
    pprintf(&Errs, "%s\n", str_val(message));
    print_stack_trace(rls);
    pprintf(&Errs, "exiting due to previous error.\n");
    exit(1);
  }

  Ctl* ctl = get_error_state(rls);

  if ( ctl == NULL ) {
    pprintf(&Errs, "%s\n", str_val(message));
    print_stack_trace(rls);
    pprintf(&Errs, "exiting because control reached toplevel without an error handler.\n");
    exit(1);
  }

  if ( !is_user_ctl(ctl) ) {
    pprintf(&Errs, "%s\n\n", str_val(message));
    print_stack_trace(rls);
    // #ifdef RASCAL_DEBUG
    // stack_report(rls, -1, "full stack at error");
    // #endif

  } else {
#ifdef RASCAL_DEBUG
    //pprintf(&Errs, "%s\n\n", str_val(message));
    //print_stack_trace(rls);
    //stack_report(rls, -1, "full stack at error");
#endif
    // prepare a standard error object on the stack
    stack_preserve(rls, 2, tag_obj(error_sym(etype)), tag_obj(message));
    mk_tuple_s(rls, 2);
    stack_rpopn(rls, 2);
    // stack_report(rls, -1, "full stack at user error");
  }

  rl_longjmp(rls, etype);
}

void runtime_error(RlState* rls, char* fmt, ...) {
  Port* tmp = tmp_port(rls);
  print_error_header(tmp, RUNTIME_ERROR);
  va_list va;
  va_start(va, fmt);
  pvprintf(tmp, fmt, va);
  va_end(va);
  Str* msg = get_error_message(rls, tmp);
  rascal_error(rls, RUNTIME_ERROR, false, msg);
}

void eval_error(RlState* rls, char* fmt, ...) {
  Port* tmp = tmp_port(rls);
  print_error_header(tmp, EVAL_ERROR);
  va_list va;
  va_start(va, fmt);
  pvprintf(tmp, fmt, va);
  va_end(va);
  Str* msg = get_error_message(rls, tmp);
  rascal_error(rls, EVAL_ERROR, false, msg);
}

void type_error(RlState* rls, Type* e, Expr g) {
  Port* tmp = tmp_port(rls);
  print_error_header(tmp, EVAL_ERROR);
  print_type_error(tmp, e, g);
  Str* msg = get_error_message(rls, tmp);

  rascal_error(rls, EVAL_ERROR, false, msg);
}

void arity_error(RlState* rls, bool va, int e, int g) {
  Port* tmp = tmp_port(rls);
  print_error_header(tmp, EVAL_ERROR);
  print_arity_error(tmp, "arguments to #", va, e, g);
  Str* msg = get_error_message(rls, tmp);
  rascal_error(rls, EVAL_ERROR, false, msg);
}

void syntax_error(RlState* rls, List* form, char* fmt, ...) {
  Port* tmp = tmp_port(rls);
  print_syntax_error_header(tmp, form);
  va_list va;
  va_start(va, fmt);
  pvprintf(tmp, fmt, va);
  va_end(va);
  Str* msg = get_error_message(rls, tmp);
  rascal_error(rls, EVAL_ERROR, false, msg);
}

void syntax_type_error(RlState* rls, List* form, Type* e, Expr g) {
  Port* tmp = tmp_port(rls);
  print_syntax_error_header(tmp, form);
  print_type_error(tmp, e, g);
  Str* msg = get_error_message(rls, tmp);
  rascal_error(rls, EVAL_ERROR, false, msg);
}

void syntax_arity_error(RlState* rls, List* form, bool va, int e) {
  Port* tmp = tmp_port(rls);
  print_syntax_error_header(tmp, form);
  print_arity_error(tmp, "expressions", va, e, form->tail->count-1);
  Str* msg = get_error_message(rls, tmp);
  rascal_error(rls, EVAL_ERROR, false, msg);
}

void system_error(RlState* rls, char* fmt, ...) {
  Port* tmp = tmp_port(rls);
  print_error_header(tmp, SYSTEM_ERROR);
  va_list va;
  va_start(va, fmt);
  pvprintf(tmp, fmt, va);
  Str* msg = get_error_message(rls, tmp);
  rascal_error(rls, SYSTEM_ERROR, false, msg);
}

void fatal_error(RlState* rls, char* fmt, ...) {
  Port* tmp = tmp_port(rls);
  print_error_header(tmp, SYSTEM_ERROR);
  va_list va;
  va_start(va, fmt);
  pvprintf(tmp, fmt, va);
  Str* msg = get_error_message(rls, tmp);
  rascal_error(rls, SYSTEM_ERROR, true, msg);
}

// miscellaneous helpers
static void print_method_trace(Port* ios, Method* m, instr_t* pc) {
  static char* user_fmt = "in file %s, in %s, near line %d.\n";
  static char* builtin_fmt = "in <builtin> %s.\n";
  static char* macro_fmt = "in file %s, in macro %s.\n"; // line information can be unreliable for macros

  if ( is_builtin_method(m) )
    pprintf(ios, builtin_fmt, method_name(m));

  else if ( m->fun->macro )
    pprintf(&Errs, macro_fmt,
            method_file(m), method_name(m));

  else
    pprintf(&Errs, user_fmt,
            method_file(m), method_name(m),
            get_line_number(m->chunk, pc));
}

void print_stack_trace(RlState* rls) {
  pprintf(&Errs, "\n\n=== backtrace ===\n\n");

  if ( rls->exec == NULL ) // reached top level
    return;

  print_method_trace(&Errs, rls->exec, rls->pc);

  for ( FrameRef f=rls->f_top-1; f > rls->frames; f-- )
    print_method_trace(&Errs, f->exec, f->pc);

  pprintf(&Errs, "\n\n");
}

char* error_name(Status etype) {
  assert(etype != OKAY);
  return ErrorNames[etype];
}

Sym* error_sym(Status etype) {
  assert(etype != OKAY);
  return ErrorSyms[etype];
}

Sym EvalError = {
  .type=&SymType,
  .gray=true,
  .nosweep=true
};

Sym RuntimeError = {
  .type=&SymType,
  .gray=true,
  .nosweep=true
};

Sym SystemError = {
  .type=&SymType,
  .gray=true,
  .nosweep=true
};

void init_vm_error(void) {
  // initialize EvalError
  EvalError.val  = mk_str(&Main, ":eval-error");
  EvalError.hashcode = hash_word(EvalError.val->hash);

  // add to permanent heap and ErrorSyms array
  add_to_permanent(&Main, &EvalError);
  ErrorSyms[EVAL_ERROR] = &EvalError;
  ErrorSyms[USER_ERROR] = &EvalError;

  // initialize RuntimeError
  RuntimeError.val  = mk_str(&Main, ":runtime-error");
  RuntimeError.hashcode = hash_word(RuntimeError.val->hash);

  // add to permanent heap and ErrorSyms array
  add_to_permanent(&Main, &RuntimeError);
  ErrorSyms[RUNTIME_ERROR] = &RuntimeError;

  // initialize SystemError
  SystemError.val  = mk_str(&Main, ":system-error");
  SystemError.hashcode = hash_word(SystemError.val->hash);

  // add to permanent heap and ErrorSyms array
  add_to_permanent(&Main, &SystemError);
  ErrorSyms[SYSTEM_ERROR] = &SystemError;
}
