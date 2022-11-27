#include "obj/nul.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
struct type_t NulType =
  {
    {
      .type=&TypeType.data,
      .size=sizeof(struct  type_t)
    },
    {
      .name="nul",
      .vmtype=vmtype_nulptr,
      .obsize=0,
      .elsize=0,
      .stringp=false
    }
  };

/* API */

/* runtime */
void rl_obj_nul_init( void ) {}

/* convenience */
