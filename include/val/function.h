#ifndef rl_val_function_h
#define rl_val_function_h

#include "runtime.h"

#include "val/object.h"

/* Types, APIs, and globals for Rascal functions. */
/* C types */

#define FN_HEADER                               \
  HEADER;                                       \
  Sym*   name;                                  \
  size32 arity;                                 \
  bool   variadic

typedef struct Fn {
  FN_HEADER;
} Fn;

struct NativeFn {
  FN_HEADER;

  Opcode    label;   // if non-zero, this function's action is an inlineable VM label
  BuiltinFn builtin; // if non-null, call this function with the vm and arity as arguments
};

struct UserFn {
  FN_HEADER;

  
};

#endif
