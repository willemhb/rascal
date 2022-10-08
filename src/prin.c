#include "prin.h"


// lisp IO
value_t lisp_prin(stream_t *port, value_t value)
{
  assert(port_outsp(port));
  
  type_t type = rtypeof(val);

  if (Print[type])
    Print[type](port, val);

  else
    port_prinf(port, "<%s>", TypeNames[type] );

  return val;
}
