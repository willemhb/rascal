#ifndef rl_vm_obj_h
#define rl_vm_obj_h

/* runtime dispatch module */

#include "vm/obj/heap.h"
#include "vm/obj/reader.h"
#include "vm/obj/vm.h"
#include "vm/obj/support/runtime.h"

/* runtime dispatch */
void rl_vm_obj_init( void );
void rl_vm_obj_mark( void );
void rl_vm_obj_cleanup( void );

#endif
