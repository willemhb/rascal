#ifndef data_upvalue_h
#define data_upvalue_h

#include "data/object.h"

// C types
struct upvalue {
  HEADER;
  variable_t* var;
  value_t*    location;
  value_t     value;
  upvalue_t*  next;
};

// APIs & utilities

#endif
