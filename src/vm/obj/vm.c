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
value_t get_toplevel_var( symbol_t *name )
{
  value_t val = ns_mapping_get(Vm.toplevel_names->locals, name);

  if ( is_fixnum(val) )
    return vector_ref(Vm.toplevel_binds->binds, as_fixnum(val));

  return val;
}

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
