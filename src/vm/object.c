#include <assert.h>
#include <string.h>

#include "vm/object.h"

#include "vm/memory.h"

/* commentary */

/* C types */

/* globals */
object_t object_type( rl_object_t *object )
{
  if (object)
    return object->object_type;

  return nul_object;
}

/* API */

/* runtime */
void rl_vm_object_init( void )    {}
void rl_vm_object_mark( void )    {}
void rl_vm_object_cleanup( void ) {}

/* convenience */
