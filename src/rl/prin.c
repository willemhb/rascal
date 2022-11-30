#include "obj/real.h"
#include "obj/stream.h"
#include "obj/type.h"

#include "rl/prin.h"


/* commentary */

/* C types */

/* globals */

/* API */
void prin( stream_t stream, value_t x )
{
  if (is_real(x))
    fprintf(stream, "%.2f", as_real(x));

  else
    {
      datatype_t *xtype = rl_typeof(x);
      fprintf(stream, "<%s>", xtype->type.name);
    }
}

void prinln( stream_t stream, value_t x )
{
  prin(stream, x);
  fprintf(stream, "\n");
}

/* runtime */
void rl_rl_prin_init( void ) {}
void rl_rl_prin_mark( void ) {}

/* convenience */
