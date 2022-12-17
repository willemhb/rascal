#ifndef rl_vm_obj_support_namespace_h
#define rl_vm_obj_support_namespace_h

#include "rascal.h"

/* commentary */

/* C types */
typedef struct namespace_t namespace_t;

/* globals */

/* API */

/* runtime dispatch */
void rl_vm_obj_support_namespace_init( void );
void rl_vm_obj_support_namespace_mark( void );
void rl_vm_obj_support_namespace_cleanup( void );

/* convenience */

#endif
