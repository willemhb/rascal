#include "obj/real.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
struct type_t RealType =
{
  {
    .type=&TypeType.data,
    .size=sizeof(struct type_t)
  },

  {
    .name="real",
    .vmtype=vmtype_flo64,
    .obsize=0,
    .elsize=0,
    .stringp=false
  }
};

/* API */

/* runtime */
void rl_obj_real_init( void ) {}

/* convenience */
