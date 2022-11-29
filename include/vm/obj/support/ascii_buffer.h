#ifndef rl_vm_obj_support_ascii_buffer_h
#define rl_vm_obj_support_ascii_buffer_h

#include "tpl/decl/buffer.h"

/* commentary

   Buffer type for ascii encoded IO and string operations. */

/* instantiations */
BUFFER(ascii_buffer, ascii_t);

/* C types */

/* globals */

/* API */

/* runtime */
void rl_vm_obj_support_ascii_buffer_init( void );
void rl_vm_obj_support_ascii_buffer_mark( void );

/* convenience */

#endif
