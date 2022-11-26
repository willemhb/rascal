#include "obj/type.h"


/* commentary */

/* C types */

/* globals */

struct type_t TypeType =
  {
    { .type=&TypeType.data, .size=sizeof(struct type_t) },

    {
      .name="type",
      .vmtype=vmtype_object,
      .obsize=sizeof(struct type_t),
      .elsize=0,
      .stringp=false
    }
  };

/* API */

/* runtime */
void rl_obj_type_init( void ) {}

/* convenience */
