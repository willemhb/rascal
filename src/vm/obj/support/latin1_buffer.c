#include "vm/obj/support/latin1_buffer.h"

#include "tpl/impl/buffer.h"

/* commentary */

/* instantiations */

BUFFER(latin1_buffer, latin1_t, uint, pad_buffer_size);

/* C types */

/* globals */

/* runtime */
void rl_vm_obj_support_latin1_buffer_init( void )    {}
void rl_vm_obj_support_latin1_buffer_mark( void )    {}
void rl_vm_obj_support_latin1_buffer_cleanup( void ) {}

/* convenience */
