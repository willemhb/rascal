#ifndef runtime_error_h
#define runtime_error_h

#include <stdarg.h>

#include "base/value.h"
#include "base/type.h"

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

// 

// utilities ------------------------------------------------------------------
void argco(char* func, int argc, bool vargs, int nargs);
void argtype(char* func, Val x, Type type);
void argval(char* func, Val x, bool (*guard)(Val v));
void argtypes(char* func, Val x, usize n, ...);

#endif
