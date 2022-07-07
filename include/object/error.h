#ifndef rascal_error_h
#define rascal_error_h

#include "rascal.h"

struct error_t {
  object_t base;
  value_t irritant;
  char_t message[1];
};

// api ------------------------------------------------------------------------
value_t trace_error(value_t errval);
value_t print_error(value_t error);

value_t mk_error( value_t irritant, char_t *message );

#endif
