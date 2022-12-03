#include "obj/bool.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
datatype_t BoolType =
  {
    { gl_datatype_head, "bool", datatype_isa, NULL },
    
    .vmtype=vmtype_boolean,
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
