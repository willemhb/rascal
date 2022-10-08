#include "number.h"
#include "port.h"

void prin_real(stream_t *port, value_t value)
{
  port_prinf( port, "%.2F", as_real(val));
}

// character
#include "chrnames.h"

void prin_chr(stream_t *port, value_t value)
{
  if (val == EOS)
    port_prinf( port, "<eos>");

  else
    port_prinf( port, "\\%s", ChrNames[as_chr(val)&128]);
}

