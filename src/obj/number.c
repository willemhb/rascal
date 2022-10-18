#include "number.h"
#include "stream.h"

void prin_real(stream_t *port, value_t value)
{
  port_prinf( port, "%.2F", as_real(value));
}

// character
#include "chrnames.h"

void prin_chr(stream_t *port, value_t value)
{
  if (value == EOS_VAL)
    port_prinf( port, "<eos>");

  else
    port_prinf( port, "\\%s", ChrNames[as_chr(value)&128]);
}

// builtin arithmetic
value_t 
