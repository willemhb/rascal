#ifndef rl_error_h
#define rl_error_h

#include "common.h"

/* status codes and basic error handling. */
typedef enum {
  /* No error */
  OKAY,

  /* error codes */
  /* error codes that may or may not imply a user mistake */
  SYSTEM_ERROR,     // error originating from OS, eg file not found. Usually fatal
  RUNTIME_ERROR,    // error originating from runtime, eg stack overflow

  /* error codes that always imply a user mistake */
  READ_ERROR,       // error originating from reader, eg unclosed '('
  COMPILE_ERROR,    // error originating from compiler, eg 
  SYNTAX_ERROR,     // error originating from compiler specifically to do with
  METHOD_ERROR,     // error originating from missing method signature
  EVAL_ERROR,       // error originating from interpreter, eg a failed type check
  USER_ERROR,       // error raised by the user
} rl_err_t;

/* Error APIs */
char*    rl_err_name(rl_err_t e);
rl_err_t rl_error(rl_err_t e, const char* fn, const char* fmt, ...);
void     rl_fatal_err(rl_err_t e, const char* fn, const char* fmt, ...);

#endif
