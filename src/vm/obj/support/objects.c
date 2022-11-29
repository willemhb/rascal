#include "vm/obj/support/objects.h"

#include "tpl/impl/alist.h"

/* commentary */

/* instantiations */
ALIST(objects, object_t*, object_t*, pad_alist_size);

/* runtime */
void rl_vm_obj_support_objects_init( void ) {}
void rl_vm_obj_support_objects_mark( void ) {}
