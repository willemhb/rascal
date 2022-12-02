#include "vm/object.h"

#include "vm/obj/vm.h"

#include "vm/obj/support/namespc.h"
#include "vm/obj/support/envt.h"

#include "obj/fixnum.h"

/* commentary */

/* C types */

/* globals */
vm_t Vm =
  {
   .panicking=false,
   .executing=NULL,
   .toplevel_names=NULL,
   .toplevel_binds=NULL
  };

/* API */

/* runtime */
void rl_vm_obj_vm_init( void )
{
  Vm.toplevel_names=make_namespc();
  Vm.toplevel_binds=make_envt();

  init_namespc(Vm.toplevel_names, NULL);
  init_envt(Vm.toplevel_binds, NULL, 0, NULL);
}

void rl_vm_obj_vm_mark( void )
{
  mark_object((object_t*)Vm.executing);
}

void rl_vm_obj_vm_cleanup( void ) {}

/* convenience */
