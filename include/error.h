#ifndef rascal_error_h
#define rascal_error_h

#include "core.h"

struct error_t {
  object_t base;
  value_t irritant;
  char_t message[1];
};

// api ------------------------------------------------------------------------
void error( value_t irritant, char_t *fmt, ...);
void require( bool_t test, char_t *fmt, ...);

#endif
