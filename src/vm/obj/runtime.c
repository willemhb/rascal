/* runtime dispatch module */
#include "vm/obj/runtime.h"


void rl_vm_obj_init( void )
{
  rl_vm_obj_heap_init();
  rl_vm_obj_reader_init();
  rl_vm_obj_vm_init();
  rl_vm_obj_support_init();
}

void rl_vm_obj_mark( void )
{
  rl_vm_obj_heap_mark();
  rl_vm_obj_reader_mark();
  rl_vm_obj_vm_mark();
  rl_vm_obj_support_mark();
}

void rl_vm_obj_cleanup( void )
{
  rl_vm_obj_heap_cleanup();
  rl_vm_obj_reader_cleanup();
  rl_vm_obj_vm_cleanup();
  rl_vm_obj_support_cleanup();
}
