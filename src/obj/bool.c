#include "obj/bool.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
vtable_t BoolMethods =
  {
    NULL, NULL, NULL
  };

layout_t BoolLayout =
  {
    .vmtype=vmtype_boolean
  };

datatype_t BoolType =
  {
    {
      .obj=obj_init(&TypeType, sizeof(datatype_t), object_fl_static),
      .name="bool"
    },

    .layout=&BoolLayout,
    .methods=&BoolMethods
  };

/* API */
bool as_cbool( value_t x )
{
  /* convert rascal value to C boolean equivalent.

     all values except nul and false are considered true. */

  return x != NUL && x != FALSE;
}

/* runtime */
void rl_obj_bool_init( void )
{
  gl_init_type(BoolType);
}

void rl_obj_bool_mark( void )
{
  gl_mark_type(BoolType);
}

void rl_obj_bool_cleanup( void ) {}
