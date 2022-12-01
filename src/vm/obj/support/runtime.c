#include "vm/obj/support/runtime.h"


/* runtime dispatch module */

/* runtime dispatch */
void rl_vm_obj_support_init( void )
{
  rl_vm_obj_support_ascii_buffer_init();
  rl_vm_obj_support_bytecode_init();
  rl_vm_obj_support_envt_init();
  rl_vm_obj_support_gc_object_frame_init();
  rl_vm_obj_support_gc_value_frame_init();
  rl_vm_obj_support_latin1_buffer_init();
  rl_vm_obj_support_namespc_init();
  rl_vm_obj_support_objects_init();
  rl_vm_obj_support_readtable_init();
  rl_vm_obj_support_stack_init();
  rl_vm_obj_support_string_init();
  rl_vm_obj_support_symbol_table_init();
  rl_vm_obj_support_utf16_buffer_init();
  rl_vm_obj_support_utf32_buffer_init();
  rl_vm_obj_support_utf8_buffer_init();
  rl_vm_obj_support_values_init();
  rl_vm_obj_support_vector_init();
}

void rl_vm_obj_support_mark( void )
{
  rl_vm_obj_support_ascii_buffer_mark();
  rl_vm_obj_support_bytecode_mark();
  rl_vm_obj_support_envt_mark();
  rl_vm_obj_support_gc_object_frame_mark();
  rl_vm_obj_support_gc_value_frame_mark();
  rl_vm_obj_support_latin1_buffer_mark();
  rl_vm_obj_support_namespc_mark();
  rl_vm_obj_support_objects_mark();
  rl_vm_obj_support_readtable_mark();
  rl_vm_obj_support_stack_mark();
  rl_vm_obj_support_string_mark();
  rl_vm_obj_support_symbol_table_mark();
  rl_vm_obj_support_utf16_buffer_mark();
  rl_vm_obj_support_utf32_buffer_mark();
  rl_vm_obj_support_utf8_buffer_mark();
  rl_vm_obj_support_values_mark();
  rl_vm_obj_support_vector_mark();
}

void rl_vm_obj_support_cleanup( void )
{
  rl_vm_obj_support_ascii_buffer_cleanup();
  rl_vm_obj_support_bytecode_cleanup();
  rl_vm_obj_support_envt_cleanup();
  rl_vm_obj_support_gc_object_frame_cleanup();
  rl_vm_obj_support_gc_value_frame_cleanup();
  rl_vm_obj_support_latin1_buffer_cleanup();
  rl_vm_obj_support_namespc_cleanup();
  rl_vm_obj_support_objects_cleanup();
  rl_vm_obj_support_readtable_cleanup();
  rl_vm_obj_support_stack_cleanup();
  rl_vm_obj_support_string_cleanup();
  rl_vm_obj_support_symbol_table_cleanup();
  rl_vm_obj_support_utf16_buffer_cleanup();
  rl_vm_obj_support_utf32_buffer_cleanup();
  rl_vm_obj_support_utf8_buffer_cleanup();
  rl_vm_obj_support_values_cleanup();
  rl_vm_obj_support_vector_cleanup();
}
