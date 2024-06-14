#include <stdlib.h>

#include "error.h"


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
}
