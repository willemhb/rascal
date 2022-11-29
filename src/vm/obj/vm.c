#include "vm/object.h"

#include "vm/obj/vm.h"

/* commentary */

/* C types */

/* globals */
vm_t Vm = { false, NULL };

/* API */

/* runtime */
void rl_vm_vm_init( void )
{
  
}

void rl_vm_vm_mark( void )
{
  mark_object((object_t*)Vm.executing);
}

/* convenience */
