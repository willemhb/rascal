#include "vm/object.h"

#include "vm/obj/support/objects.h"

#include "tpl/impl/alist.h"

/* commentary */

/* instantiations */
ALIST(objects, object_t*, object_t*, pad_alist_size);

/* API */
void mark_objects( objects_t *objects )
{
  for ( size_t i=0; i<objects->len; i++ )
    mark_object(alist_member(objects, i, object_t*));
}

/* runtime */
void rl_vm_obj_support_objects_init( void ) {}
void rl_vm_obj_support_objects_mark( void ) {}
