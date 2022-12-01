#ifndef rl_vm_obj_support_latin1_buffer_h
#define rl_vm_obj_support_latin1_buffer_h

#include "tpl/decl/buffer.h"

/* commentary

   Buffer type for latin1 encoded IO and string operations. */

/* instantiations */
BUFFER(latin1_buffer, latin1_t);

/* C types */

/* globals */

/* API */

/* runtime */
void rl_vm_obj_support_latin1_buffer_init( void );
void rl_vm_obj_support_latin1_buffer_mark( void );
void rl_vm_obj_support_latin1_buffer_cleanup( void );

/* convenience */

#endif
