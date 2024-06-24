#include <stdlib.h>

#include "error.h"

#include "val/type.h"

char* rl_err_name(rl_err_t s) {
  static char* names[] = {
    [OKAY]          = "okay",
    [SYSTEM_ERROR]  = "system-error",
    [RUNTIME_ERROR] = "runtime-error",
    [READ_ERROR]    = "read-error",
    [COMPILE_ERROR] = "compile-error",
    [METHOD_ERROR]  = "method-error",
    [EVAL_ERROR]    = "eval-error",
    [USER_ERROR]    = "user-error"
  };

  return names[s];
}

rl_err_t rl_error(rl_err_t e, const char* fn, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  fprintf(stderr, "%s:%s: ", rl_err_name(e), fn);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  va_end(va);

  return e;
}

void rl_fatal_err(rl_err_t e, const char* fn, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  fprintf(stderr, "%s:%s: ", rl_err_name(e), fn);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  va_end(va);

  exit(1);
}

/* Error checking helpers. */
rl_err_t check_argc(rl_err_t e, const char* fn, size_t x, size_t g, bool v) {
  static const char* afmt = "expected %zu inputs, got %zu";
  static const char* vfmt = "expected at least %zu inputs, got %zu";

  rl_err_t o = OKAY;

  if ( v && x < g )
    o = rl_error(e, fn, vfmt, x, g);

  else if ( x != g )
    o = rl_error(e, fn, afmt, x, g);

  return o;
}

rl_err_t rl_type(rl_err_t e, const char* fn, Type* x, Type* g) {
  static const char* fmt = "Expected value to have type %s, has type %s";

  rl_err_t o = OKAY;

  if ( !has_instance(x, g) )
    o = rl_error(e, fn, fmt, t_name(x), t_name(g));

  return o;
}
