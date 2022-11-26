#include "obj/stream.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
struct type_t StreamType =
  {
    {
      .type=&TypeType.data,
      .size=sizeof(struct type_t)
    },

    {
      .name="stream",
      .vmtype=vmtype_object,
      .obsize=sizeof(stream_t),
      .elsize=0,
      .stringp=false
    }
  };

stream_t Ins, Outs, Errs;

/* API */


/* runtime */
void rl_obj_stream_init( void )
{
  Ins  = stdin;
  Outs = stdout;
  Errs = stderr;
}

/* convenience */
