#ifndef rl_status_h
#define rl_status_h

#include "common.h"

/* status codes and basic error handling. */
typedef enum {
  /* default return value */
  OKAY,

  /* table lookup status codes */
  NOTFOUND,
  ADDED,
  UPDATED,
  REMOVED,

  /* reader status codes */
  READY,
  EXPRESSION,
  END_OF_INPUT,

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
} rl_status_t;

/* Error APIs */
bool        is_error_status(rl_status_t s);
const char* rl_status_name(rl_status_t s);
rl_status_t rl_error(rl_status_t code, const char* fname, const char* fmt, ...);
void        rl_fatal_error(rl_status_t code, const char* fname, const char* fmt, ...);

#endif
