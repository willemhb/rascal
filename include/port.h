#ifndef rascal_port_h
#define rascal_port_h

#include <stdio.h>

#include "object.h"

// C types --------------------------------------------------------------------
typedef enum
  {
    port_fl_in  = 1,
    port_fl_out = 2,
    port_fl_text= 4,
    port_fl_bin = 8,
    port_fl_lisp=12
  } port_fl_t;

struct port_t
{
  HEADER;

  value_t   value;
  object_t *buffer;
  FILE     *ios;
};

// forward declarations -------------------------------------------------------

#endif
