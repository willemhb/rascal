#ifndef data_closure_h
#define data_closure_h

#include "data/object.h"

// C types
struct closure {
  HEADER;
  function_t* function;
  list_t*     envt;
};

// APIs & utilities

#endif
