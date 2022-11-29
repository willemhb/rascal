#ifndef rl_vm_obj_support_utf16_buffer_h
#define rl_vm_obj_support_utf16_buffer_h

#include "tpl/decl/buffer.h"

/* commentary

   Buffer type for utf16 encoded IO and string operations. */

/* instantiations */
BUFFER(utf16_buffer, utf16_t);

/* C types */

/* globals */

/* API */

/* runtime */
void rl_vm_obj_support_utf16_buffer_init( void );
void rl_vm_obj_support_utf16_buffer_mark( void );

/* convenience */

#endif
