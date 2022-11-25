#include "vm/vm.h"
#include "obj/closure.h"
#include "obj/control.h"

/* globals */
vm_t Vm;

/* runtime */
void rl_vm_vm_init( void )
{
  Vm.error      = NULL;
  Vm.panic_mode = false;
  Vm.control    = NULL;
  Vm.toplevel   = NULL;
}

void rl_vm_vm_mark( void )
{
  
}
