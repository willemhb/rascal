#include "prin.h"


// lisp IO
val_t lisp_prin(port_t *port, val_t val)
{
  assert(port_outsp(port));
  
  type_t type = rtypeof(val);

  if (Print[type])
    Print[type](port, val);

  else
    port_prinf(port, "<%s>", TypeNames[type] );

  return val;
}
