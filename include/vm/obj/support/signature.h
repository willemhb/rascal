#ifndef rl_vm_obj_support_signature_h
#define rl_vm_obj_support_signature_h

#include "rascal.h"

/* commentary

   A fixed array of types. Used in many places.
 */

/* C types */
typedef type_t **signature_t;

/* globals */

/* API */

/* runtime dispatch */
void rl_vm_obj_support_signature_init( void );
void rl_vm_obj_support_signature_mark( void );
void rl_vm_obj_support_signature_cleanup( void );

/* convenience */

#endif
