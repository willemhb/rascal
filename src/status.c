#include <stdlib.h>

#include "status.h"


bool is_error_status(rl_status_t s) {
  return s >= SYSTEM_ERROR;
}

const char* rl_status_name(rl_status_t s) {
  static const char* names[] = {
    [OKAY]          = "okay",
    [NOTFOUND]      = "notfound",
    [ADDED]         = "added",
    [UPDATED]       = "updated",
    [REMOVED]       = "removed",
    [READY]         = "ready",
    [EXPRESSION]    = "expression",
    [END_OF_INPUT]  = "end-of-input",
    [SYSTEM_ERROR]  = "system-error",
    [RUNTIME_ERROR] = "runtime-error",
    [READ_ERROR]    = "read-error",
    [COMPILE_ERROR] = "compile-error",
    [EVAL_ERROR]    = "eval-error",
    [USER_ERROR]    = "user-error"
  };

  return names[s];
}

rl_status_t rl_error(rl_status_t code, const char* fname, const char* fmt, ...) {
  va_list va;
  va_start(va, fmt);
  fprintf(stderr, "%s:%s: ", rl_status_name(code), fname);
  vfprintf(stderr, fmt, va);
  fprintf(stderr, ".\n");
  va_end(va);

  if ( code == SYSTEM_ERROR ) // not recoverable
    exit(1);

  return code;
}
