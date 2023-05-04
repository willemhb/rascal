#ifndef data_native_h
#define data_native_h

#include "data/object.h"

// C types
struct native {
  HEADER;
  symbol_t* name;
  uint64    arity;
  value_t (*callback)(usize n, value_t* args);
};

// APIs & utilities
#define is_native(x) rl_isa(x, NATIVE)
#define as_native(x) rl_asa(x, WVMASK, native_t*)

#endif
