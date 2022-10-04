#include "number.h"
#include "port.h"

void prin_real(port_t *port, val_t val)
{
  port_prinf( port, "%.2F", as_real(val));
}

// character
#include "chrnames.h"

void prin_chr(port_t *port, val_t val)
{
  if (val == EOS)
    port_prinf( port, "<eos>");

  else
    port_prinf( port, "\\%s", ChrNames[as_chr(val)&128]);
}

