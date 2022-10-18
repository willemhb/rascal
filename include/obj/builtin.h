#ifndef rascal_builtin_h
#define rascal_builtin_h

#include "obj/cvalue.h"
#include "rl/invoke.h"


#define NATIVE_FN(ftype)			\
  struct rl_##ftype##_t				\
  {						\
    cvalue_t  cvalue;				\
    ftype##_t callback;				\
  }

// wrapper types for builtin functions
NATIVE_FN(isa);
NATIVE_FN(has);
NATIVE_FN(thunk);
NATIVE_FN(unary);
NATIVE_FN(binary);
NATIVE_FN(ternary);
NATIVE_FN(nary);
NATIVE_FN(compare);
NATIVE_FN(hashof);
NATIVE_FN(sizeof);
NATIVE_FN(read);
NATIVE_FN(print);
NATIVE_FN(intern);
NATIVE_FN(construct);
NATIVE_FN(obinit);
NATIVE_FN(trace);
NATIVE_FN(free);
NATIVE_FN(resize);
NATIVE_FN(cvinit);
NATIVE_FN(unbox);
NATIVE_FN(invoke);

#endif
