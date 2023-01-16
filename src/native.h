#ifndef rl_native_h
#define rl_native_h

#include "func.h"

/* C types */
/* globals */
extern struct type_t NativeType;

/* convenience */
#define is_native(x) has_type(x, &NativeType)
#define as_native(x) ((native_t)data_of(x))

#define native_method(name, nargs, vargs, guard, fn)			\
  do {									\
    val_t __fn__ = tag_val(fn, NATIVE);					\
    									\
  } while (false)

#endif
