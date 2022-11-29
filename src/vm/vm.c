#include "vm/vm.h"

/* commentary */

/* C types */

/* globals */

/* API */

/* runtime */
void rl_vm_init( void )
{
  rl_vm_obj_heap_init();
  rl_vm_obj_reader_init();
  rl_vm_obj_vm_init();

  rl_vm_error_init();
  rl_vm_memory_init();
  rl_vm_value_init();
  rl_vm_object_init();

  rl_vm_obj_support_ascii_buffer_init();
  rl_vm_obj_support_latin1_buffer_init();
  rl_vm_obj_support_utf8_buffer_init();
  rl_vm_obj_support_utf16_buffer_init();
  rl_vm_obj_support_utf32_buffer_init();

  rl_vm_obj_support_objects_init();
  rl_vm_obj_support_values_init();
  rl_vm_obj_support_stack_init();

  rl_vm_obj_support_string_init();
  rl_vm_obj_support_bytecode_init();
  rl_vm_obj_support_vector_init();

  rl_vm_obj_support_readtable_init();
  rl_vm_obj_support_symbol_table_init();
}

void rl_vm_mark( void )
{
  rl_vm_obj_heap_mark();
  rl_vm_obj_reader_mark();
  rl_vm_obj_vm_mark();

  rl_vm_error_mark();
  rl_vm_memory_mark();
  rl_vm_value_mark();
  rl_vm_object_mark();

  rl_vm_obj_support_ascii_buffer_mark();
  rl_vm_obj_support_latin1_buffer_mark();
  rl_vm_obj_support_utf8_buffer_mark();
  rl_vm_obj_support_utf16_buffer_mark();
  rl_vm_obj_support_utf32_buffer_mark();

  rl_vm_obj_support_objects_mark();
  rl_vm_obj_support_values_mark();
  rl_vm_obj_support_stack_mark();

  rl_vm_obj_support_string_mark();
  rl_vm_obj_support_bytecode_mark();
  rl_vm_obj_support_vector_mark();

  rl_vm_obj_support_readtable_mark();
  rl_vm_obj_support_symbol_table_mark();
}

/* convenience */
