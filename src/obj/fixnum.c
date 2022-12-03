#include "obj/fixnum.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
datatype_t FixnumType =
  {
    .type   = { gl_datatype_head, "fixnum", datatype_isa, NULL },
    .vmtype = vmtype_uint64
  };

/* API */
value_t fixnum( fixnum_t x )
{
  return tag(x, FIXNUM);
}

/* runtime dispatch */
void rl_obj_fixnum_init( void )
{
  gl_init_type(FixnumType);
}

void rl_obj_fixnum_mark( void )
{
  gl_mark_type(FixnumType);
}

void rl_obj_fixnum_cleanup( void ) {}
