#include "obj/nul.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
struct vtable_t NulMethods = { NULL, NULL, NULL };

struct layout_t NulLayout =
  {
   .vmtype=vmtype_nulptr,
   .flags=0,
   .obsize=0
  };

struct datatype_t NulType =
  {
   {
    .obj=obj_init(&TypeType, sizeof(datatype_t), object_fl_static),
    .name="nul"
   },

   .methods=&NulMethods,
   .layout=&NulLayout
  };

/* API */

/* runtime */
void rl_obj_nul_init( void )
{
  gl_init_type(NulType);
}

void rl_obj_nul_mark( void )
{
  gl_mark_type(NulType);
}

void rl_obj_nul_cleanup( void ) {}

/* convenience */
