#ifndef rl_obj_function_h
#define rl_obj_function_h

#include "rascal.h"

#include "vm/object.h"

#include "vm/obj/support/method_table.h"

/* commentary

   a rascal generic function. */

/* C types */
struct function_t
{
  rl_object_t     obj;

  string_t        name;
  method_table_t *methods;
};

/* globals */


/* API */

/* runtime dispatch */
void rl_obj_function_init( void );
void rl_obj_function_mark( void );
void rl_obj_function_cleanup( void );

/* convenience */

#endif
