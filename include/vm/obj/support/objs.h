#ifndef rl_vm_obj_support_objs_h
#define rl_vm_obj_support_objs_h

#include "tpl/decl/alist.h"

/* commentary

   Alist type for untagged objects (various and sundry applications in
   memory management and for internal types). */

/* instantiations */
ALIST(objs, obj_t*);

/* C types */

/* globals */

/* API */
void mark_objects( objs_t *objects );

/* runtime */
void rl_vm_obj_support_objs_init( void );
void rl_vm_obj_support_objs_mark( void );
void rl_vm_obj_support_objs_cleanup( void );

/* convenience */

#endif
