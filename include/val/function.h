#ifndef rl_val_function_h
#define rl_val_function_h

#include "val/object.h"

/* types for implementing rascal's multimethods. */

/* C types */
typedef Value (*NativeFn)(size_t n, Value* a);

struct GenericFn {
  HEADER;
  MutDict* cache;
  Obj*     methods;
};

struct Native {
  HEADER;
  NativeFn func;
};

struct Closure {
  HEADER;
  Chunk*   code;
  Objects* upvals;
};

/* Globals */
extern Type GenericFnType, NativeType, ClosureType;

/* External API */

#endif
