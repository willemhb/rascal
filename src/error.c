#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "error.h"

#include "vm/state.h"

#include "val/environ.h"
#include "val/type.h"
#include "val/text.h"

#include "lang/print.h"

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

Str* rl_err_str(Error e, const char* fn, const char* fmt, ...) {
  Str* r;
  va_list va;

  va_start(va, fmt);
  r = rl_verr_str(e, fn, fmt, va);
  va_end(va);

  return r;
}

Str* rl_verr_str(Error e, const char* fn, const char* fmt, va_list va) {
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
void rlp_prn_err(RlProc* p) {
  if ( p->err != OKAY ) {
    rl_prf(&StdErr, p->errm);
    rl_prf(&StdErr, ".\n");

    if ( p->errb != NOTHING ) {
      rl_prf(&StdErr, "blamed on: ");
      rl_prval(&StdErr, p->errb);
      rl_prf(&StdErr, "\n");
    }
  }
}

Error rlp_panic(RlProc* p, Error e, Val b, const char* fn, const char* fmt, ...) {
  if ( p->err == OKAY ) {

    if ( fn == NULL )
      fn = rlp_fname(p);

    va_list va;
    va_start(va, fmt);

    p->err  = e;
    p->errm = rl_verr_str(e, fn, fmt, va);
    p->errb = b;
    
    va_end(va);
  }

  return p->err;
}

Error rlp_recover(RlProc* p, Sym** k, Str** m, Val* b) {
  Error o = p->err;
  
  if ( p->err ) {
    if ( m )
      *m = p->errm;

    p->errm = NULL;

    if ( k && p->err != USER_ERR )
      *k = rl_err_kwd(p->err);

    p->err = OKAY;

    if ( b )
      *b = p->errb;

    p->errb = NOTHING;
  }

  return o;
}

Error rlp_chk_argc(RlProc* p, Error e, const char* fn, size_t x, size_t g, bool v) {
  static const char* fmt  = "expected %zu arguments to #, got %zu";
  static const char* vfmt = "expected at least %zu arguments to #, got %zu";

  if ( v && g < x )
    rlp_panic(p, e, NOTHING, fn, vfmt, x, g);

  else if ( g != x )
    rlp_panic(p, e, NOTHING, fn, fmt, x, g);

  return p->err;
}

Error rlp_chk_argt(RlProc* p, Error e, const char* fn, Type* x, Val g) {
  static const char* fmt = "Expected value of type %s, got value of type %s";

  Type* gt = type_of(g);
  
  if ( !has_instance(x, gt) )
    rlp_panic(p, e, g, fn, fmt, t_name(x), t_name(gt));

  return p->err;
}

Error rlp_chk_bound(RlProc* p, Error e, const char* fn, void* a, void* l, void* u) {
  static const char* fmt = "location %p out of bounds for bufer from %p to %p";

  if ( a < l || a > u )
    rlp_panic(p, e, NOTHING, fn, fmt, a, l, u);

  return p->err;
}

Error rlp_chk_lbound(RlProc* p, Error e, const char* fn, void* a, void* l) {
  static const char* fmt = "location %p less than lower bound of %p";

  if ( a < l )
    rlp_panic(p, e, NOTHING, fn, fmt, a, l);

  return p->err;
}

Error rlp_chk_ubound(RlProc* p, Error e, const char* fn, void* a, void* u) {
  static const char* fmt = "location %p greater than upper bound of %p";

  if ( a > u )
    rlp_panic(p, e, NOTHING, fn, fmt, a, u);

  return p->err;
}

Error rlp_chk_grows(RlProc* p, Error e, const char* fn, size_t n) {
  static const char* fmt = "stack overflow";

  if ( p->stk->cnt + n > MAX_ARITY )
    rlp_panic(p, e, NOTHING, fn, fmt);

  return p->err;
}

Error rlp_chk_push(RlProc* p, Error e, const char* fn) {
  static const char* fmt = "stack overflow";

  if ( p->stk->cnt == MAX_ARITY )
    rlp_panic(p, e, NOTHING, fn, fmt);

  return p->err;
}

Error rlp_chk_shrinks(RlProc* p, Error e, const char* fn, size_t n) {
  static const char* fmt = "stack underflow";

  if ( p->stk->cnt < n )
    rlp_panic(p, e, NOTHING, fn, fmt);

  return p->err;
}

Error rlp_chk_pop(RlProc* p, Error e, const char* fn) {
  static const char* fmt = "stack underflow";

  if ( p->stk->cnt == 0 )
    rlp_panic(p, e, NOTHING, fn, fmt);

  return p->err;
}

Error rlp_chk_sref(RlProc* p, Error e, const char* fn, long i) {
  static const char* fmt = "index %ld out of bounds for stack of size %zu";

  long _i = i < 0 ? (long)p->stk->cnt + i : i;

  if ( _i < 0 || _i > (long)p->stk->cnt )
    rlp_panic(p, e, NOTHING, fn, fmt, i, p->stk->cnt);

  return p->err;
}

/* Initialization */
void rl_error_init(void) {
  ErrorKwds[OKAY]      = mk_sym(":okay", NULL, false);
  ErrorKwds[SYS_ERR]   = mk_sym(":system-error", NULL, false);
  ErrorKwds[RUN_ERR]   = mk_sym(":runtime-error", NULL, false);
  ErrorKwds[READ_ERR]  = mk_sym(":read-error", NULL, false);
  ErrorKwds[COMP_ERR]  = mk_sym(":compile-error", NULL, false);
  ErrorKwds[GENFN_ERR] = mk_sym(":method-error", NULL, false);
  ErrorKwds[EVAL_ERR]  = mk_sym(":eval-error", NULL, false);
  ErrorKwds[USER_ERR]  = mk_sym(":user-error", NULL, false);
}
