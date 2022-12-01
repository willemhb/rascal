#ifndef rl_vm_obj_support_utf8_buffer_h
#define rl_vm_obj_support_utf8_buffer_h

#include "tpl/decl/buffer.h"

/* commentary

   Buffer type for utf8 encoded IO and string operations. */

/* instantiations */
BUFFER(utf8_buffer, utf8_t);

/* C types */

/* globals */

/* API */

/* runtime */
void rl_vm_obj_support_utf8_buffer_init( void );
void rl_vm_obj_support_utf8_buffer_mark( void );
void rl_vm_obj_support_utf8_buffer_cleanup( void );

/* convenience */

#endif
