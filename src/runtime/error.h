#ifndef runtime_error_h
#define runtime_error_h

#include <stdarg.h>

#include "types/value.h"
#include "types/type.h"

/* C types */
typedef enum ErrorType ErrorType;

enum ErrorType {
  NO_ERROR,
  READ_ERROR=4,
  COMPILE_ERROR,
  EVAL_ERROR,
  APPLY_ERROR
};

#define ERROR_BUFFER_SIZE 2048

struct Error {
  bool      panicking;
  ErrorType type;
  Val       irritant;
  char      message[ERROR_BUFFER_SIZE];
};

/* globals */
extern struct Error Error;

/* API */
bool      panicking(void);
Val       panic(char* func, ErrorType type, Val irritant, char *fmt, ...);
Val       vpanic(char* func, ErrorType type, Val irritant, char *fmt, va_list va);
ErrorType recover(void);

// utilities ------------------------------------------------------------------
#define GUARD(condition, fname, type, irr, fmt, ...)                    \
  do {                                                                  \
    if (!(condition))                                                   \
      return panic(fname, type, irr, fmt __VA_OPT__(,) __VA_ARGS__);   \
  } while (false)

#define argco(fname, argc, nargs)                               \
  GUARD(argc != nargs, fname, APPLY_ERROR, tag(nargs),          \
        "expected %du arguments, got %du", argc, nargs)

#define argtype(fname, val, type)                               \
  GUARD(has_type(val, type), fname, APPLY_ERROR, val,           \
        "wrong type: expected %s", MetaTables[type].name->name)

#define argval(fname, test, val, msg, ...)              \
  GUARD(test(val), fname, APPLY_ERROR, val,             \
        "failed test: "msg __VA_OPT__(,) __VA_ARGS__)

#endif
