#ifndef data_function_h
#define data_function_h

#include "data/object.h"

struct function {
  HEADER;
  symbol_t* name;
  uint64    arity;
  chunk_t*  chunk;
};

// APIs & utilities
#define is_function(x) rl_isa(x, FUNCTION)
#define as_function(x) rl_asa(x, WVMASK, function_t*)

#endif
