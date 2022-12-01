#include "obj/native.h"
#include "obj/type.h"
#include "obj/symbol.h"

/* commentary

   Type of builtin functions. */

/* C types */

/* globals */
struct vtable_t NativeMethods =
  {
    NULL, NULL, NULL
  };

struct layout_t NativeLayout =
  {
    .vmtype=vmtype_funptr,
    .obsize=0
  };

struct datatype_t NativeType =
  {
    {
      .obj = obj_init(&TypeType, sizeof(datatype_t), object_fl_static),
      .name= "native"
    },

    .methods=&NativeMethods,
    .layout=&NativeLayout
  };

/* API */
void define_native( char *name, native_t fn )
{
  define(name, tag(fn, NATIVE)); 
}

/* runtime */
void rl_obj_native_init( void )
{
  gl_init_type(NativeType);
}

void rl_obj_native_mark( void )
{
  gl_mark_type(NativeType);
}

void rl_obj_native_cleanup( void ) {}

/* convenience */
