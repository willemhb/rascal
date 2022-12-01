#ifndef rl_vm_obj_support_objects_h
#define rl_vm_obj_support_objects_h

#include "tpl/decl/alist.h"

/* commentary

   Alist type for untagged objects (various and sundry applications in
   memory management). */

/* instantiations */
ALIST(objects, object_t*);

/* C types */

/* globals */

/* API */
void mark_objects( objects_t *objects );

/* runtime */
void rl_vm_obj_support_objects_init( void );
void rl_vm_obj_support_objects_mark( void );
void rl_vm_obj_support_objects_cleanup( void );

/* convenience */

#endif
