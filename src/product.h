#ifndef product_h
#define product_h

// user types
#include "value.h"

struct struct_t {
  HEADER;
  object_type_t* type;
  tuple_t* slots;
};

struct record_t {
  HEADER;
  object_type_t* type;
  tuple_t* slots;
};

#endif
