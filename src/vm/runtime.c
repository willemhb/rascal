#include "vm/runtime.h"

/* commentary */

/* C types */

/* globals */

/* API */

/* runtime */
void rl_vm_init( void )
{
  rl_vm_memory_init();
  rl_vm_error_init();
  rl_vm_obj_init();
  rl_vm_value_init();
  rl_vm_object_init();
}

void rl_vm_mark( void )
{
  rl_vm_memory_mark();
  rl_vm_error_mark();
  rl_vm_obj_mark();
  rl_vm_value_mark();
  rl_vm_object_mark();
}

void rl_vm_cleanup( void )
{
  rl_vm_memory_cleanup();
  rl_vm_error_cleanup();
  rl_vm_value_cleanup();
  rl_vm_object_cleanup();
  rl_vm_obj_cleanup();
}


/* convenience */
