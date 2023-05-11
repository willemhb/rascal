#ifndef data_upvalue_h
#define data_upvalue_h

#include "data/object.h"

// C types
struct upvalue {
  HEADER;
  upvalue_t* next;
  value_t    name;
  union {
    value_t* location;
    value_t  binding;
  };
};

typedef enum {
  CLOSED = 0x01
} upvalue_fl_t;

#endif
