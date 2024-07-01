#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"

#include "vm/state.h"

#include "val/environ.h"
#include "val/type.h"
#include "val/text.h"

/* Globals */
#define EBUF_SIZE 4096

Sym* ErrorKwds[NUM_ERRORS]     = {};
char ErrorMsgBuffer[EBUF_SIZE] = {};

/* External APIs */
char* rl_err_name(Error e) {
  return rl_err_kwd(e)->name->chars+1;
}

Sym* rl_error_kwd(Error e) {
  return ErrorKwds[e];
}

Str* rl_perror(Error e, const char* fn, const char* fmt, ...) {
  Str* r;
  va_list va;

  va_start(va, fmt);
  r = rl_vperror(e, fn, fmt, va);
  va_end(va);

  return r;
}

Str* rl_vperror(Error e, const char* fn, const char* fmt, va_list va) {
  int o;
  Str* r;

  o  = snprintf(ErrorMsgBuffer, EBUF_SIZE, "%s: %s: ", fn, rl_err_name(e));
  o += vsnprintf(ErrorMsgBuffer+o, EBUF_SIZE-o, fmt, va);

  if ( o < EBUF_SIZE )
    ErrorMsgBuffer[o++] = '.';

  r  = mk_str(ErrorMsgBuffer, o, false);
  memset(ErrorMsgBuffer, 0, o);

  return r;
}

void rl_fatal_err(Error e, const char* fn, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  fprintf(stderr, "%s:%s: ", rl_err_name(e), fn);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  va_end(va);

  exit(1);
}

/* Error checking helpers. */
void rlp_panic(RlProc* p, Error e, const char* fn, const char* fmt, ...) {
  if ( p->err != OKAY )
    return;

  if ( fn == NULL )
    fn = rlp_fname(p);

  va_list va;
  va_start(va, fmt);

  p->err  = e;
  p->errm = rl_vperror(e, fn, fmt, va);

  va_end(va);
}

void rlp_argc(RlProc* p, Error e, const char* fn, size_t x, size_t g, bool v) {
  static const char* fmt  = "expected %zu arguments to #, got %zu";
  static const char* vfmt = "expected at least %zu arguments to #, got %zu";

  if ( v && g < x )
    rlp_panic(p, e, fn, vfmt, x, g);

  else if ( g != x )
    rlp_panic(p, e, fn, fmt, x, g);
}

void rlp_argt(RlProc* p, Error e, const char* fn, Type* x, Type* g) {
  static const char* fmt = "Expected value of type %s, got value of type %s";

  if ( !has_instance(x, g) )
    rlp_panic(p, e, fn, fmt, t_name(x), t_name(g));
}

void rlp_bound(RlProc* p, Error e, const char* fn, void* a, void* l, void* u) {
  static const char* fmt = "location %p out of bounds for bufer from %p to %p";

  if ( a < l || a > u )
    rlp_panic(p, e, fn, fmt, a, l, u);
}

void rlp_lbound(RlProc* p, Error e, const char* fn, void* a, void* l) {
  static const char* fmt = "location %p less than lower bound of %p";

  if ( a < l )
    rlp_panic(p, e, fn, fmt, a, l);
}

void rlp_ubound(RlProc* p, Error e, const char* fn, void* a, void* u) {
  static const char* fmt = "location %p greater than upper bound of %p";

  if ( a > u )
    rlp_panic(p, e, fn, fmt, a, u);
}

/* Initialization */
void rl_error_init(void) {
  ErrorKwds[OKAY]          = mk_sym(":okay", NULL, false);
  ErrorKwds[SYSTEM_ERROR]  = mk_sym(":system-error", NULL, false);
  ErrorKwds[RUNTIME_ERROR] = mk_sym(":runtime-error", NULL, false);
  ErrorKwds[READ_ERROR]    = mk_sym(":read-error", NULL, false);
  ErrorKwds[COMPILE_ERROR] = mk_sym(":compile-error", NULL, false);
  ErrorKwds[METHOD_ERROR]  = mk_sym(":method-error", NULL, false);
  ErrorKwds[EVAL_ERROR]    = mk_sym(":eval-error", NULL, false);
  ErrorKwds[USER_ERROR]    = mk_sym(":user-error", NULL, false);
}
