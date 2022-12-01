#include "vm/obj/support/ascii_buffer.h"

#include "tpl/impl/buffer.h"

/* commentary */

/* instantiations */

BUFFER(ascii_buffer, ascii_t, uint, pad_buffer_size);

/* C types */

/* globals */

/* runtime */
void rl_vm_obj_support_ascii_buffer_init( void )    {}
void rl_vm_obj_support_ascii_buffer_mark( void )    {}
void rl_vm_obj_support_ascii_buffer_cleanup( void ) {}

/* convenience */
