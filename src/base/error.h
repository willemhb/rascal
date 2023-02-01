#ifndef rascal_base_error_h
#define rascal_base_error_h

#include "rascal.h"

/* C types */
enum RlError {
  NoError,
  ReadError,
  CompileError,
  EvalError,
  SystemError
};

struct Errors {
  bool    panicking;
  RlError error;
};

#endif
