#ifndef rl_vm_runtime_h
#define rl_vm_runtime_h

/* runtime dispatch module */
#include "vm/error.h"
#include "vm/memory.h"
#include "vm/value.h"
#include "vm/object.h"
#include "vm/obj/runtime.h"

/* commentary

   toplevel initialization and memory management entry points.

   includes everything from this directory/subdirectories to make
   rascal main cleaner. */

void rl_vm_init( void );
void rl_vm_mark( void );
void rl_vm_clenaup( void );

#endif
