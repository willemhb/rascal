#include "obj/real.h"
#include "obj/stream.h"
#include "obj/type.h"

#include "vm/prin.h"


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
      type_t xtype = rl_typeof(x);
      fprintf(stream, "<%s>", xtype->name);
    }
}

/* runtime */
void rl_vm_prin_init( void ) {}

/* convenience */
