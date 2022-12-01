#include "obj/fixnum.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
vtable_t FixnumMethods =
  {
    NULL, NULL, NULL
  };

layout_t FixnumLayout =
  {
    .vmtype=vmtype_uint64
  };

datatype_t FixnumType =
  {
    {
      .obj=gl_type_head,
      .name="fixnum",
    },

    .layout=&FixnumLayout,
    .methods=&FixnumMethods
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

