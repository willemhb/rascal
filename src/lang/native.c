#include "error.h"

#include "lang/native.h"
#include "lang/compare.h"

#include "val/environ.h"
#include "val/function.h"

/* Globals */
#define NATIVE_OBJ(N, f)                        \
  rl_err_t f(size_t n, Val* a, Val* b);         \
                                                \
  Native N = {                                  \
    .type=&NativeType,                          \
    .gray=true,                                 \
    .C_fn=f                                     \
  }

NATIVE_OBJ(SameFn, rl_same_fn);
NATIVE_OBJ(EgalFn, rl_egal_fn);
NATIVE_OBJ(OrdFn,  rl_ord_fn);
NATIVE_OBJ(HashFn, rl_hash_fn);

#undef NATIVE_OBJ

/* Internal Native function definitions. */
rl_err_t rl_same_fn(size_t n, Val* a, Val* b) {
  rl_err_t o = chk_argc(EVAL_ERROR, "same?", 2, n, false);

  if ( o == OKAY )
    *b = rl_same(a[0], a[1]) ? TRUE : FALSE;

  return o;
}

rl_err_t rl_egal_fn(size_t n, Val* a, Val* b) {
  rl_err_t o = chk_argc(EVAL_ERROR, "=", 2, n, false);

  if ( o == OKAY )
    *b = rl_egal(a[0], a[1]) ? TRUE : FALSE;

  return o;
}

rl_err_t rl_ord_fn(size_t n, Val* a, Val* b) {
  rl_err_t o = chk_argc(EVAL_ERROR, "ord", 2, n, false);

  if ( o == OKAY )
    *b = tag(rl_order(a[0], a[1]));

  return o;
}

/* External APIs */
void rl_init_native_fns(void) {
  // create global definitions for native functions
  define("same?", &SameFn, &Globals);
  define("=", &EgalFn, &Globals);
  define("ord", &OrdFn, &Globals);
  define("hash", &HashFn, &Globals);
}
