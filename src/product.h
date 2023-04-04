#ifndef product_h
#define product_h

// user types
#include "object.h"

struct record_t {
  HEADER;
  data_type_t* type;
  dict_t* slots;
};

#endif
