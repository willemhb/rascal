#ifndef rascal_port_h
#define rascal_port_h

#include <stdio.h>
#include "core.h"

struct port_t {
  object_t base;
  uint_t flags, token;
  value_t tokval;
  ulong_t pos, line, col;
  object_t *buffer;

  FILE *ios;
  char_t name[1];
};

// api ------------------------------------------------------------------------

#endif
