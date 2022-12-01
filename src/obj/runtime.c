#include "obj/runtime.h"

/* runtime */
void rl_obj_init( void )
{
  rl_obj_bool_init();
  rl_obj_closure_init();
  rl_obj_cons_init();
  rl_obj_control_init();
  rl_obj_fixnum_init();
  rl_obj_lambda_init();
  rl_obj_native_init();
  rl_obj_nul_init();
  rl_obj_real_init();
  rl_obj_stream_init();
  rl_obj_type_init();
  rl_obj_symbol_init();
}

void rl_obj_mark( void )
{
  rl_obj_bool_mark();
  rl_obj_closure_mark();
  rl_obj_cons_mark();
  rl_obj_control_mark();
  rl_obj_fixnum_mark();
  rl_obj_lambda_mark();
  rl_obj_native_mark();
  rl_obj_nul_mark();
  rl_obj_real_mark();
  rl_obj_stream_mark();
  rl_obj_type_mark();
  rl_obj_symbol_mark();
}

void rl_obj_cleanup( void )
{
  rl_obj_bool_cleanup();
  rl_obj_closure_cleanup();
  rl_obj_cons_cleanup();
  rl_obj_control_cleanup();
  rl_obj_fixnum_cleanup();
  rl_obj_lambda_cleanup();
  rl_obj_native_cleanup();
  rl_obj_nul_cleanup();
  rl_obj_real_cleanup();
  rl_obj_stream_cleanup();
  rl_obj_type_cleanup();
  rl_obj_symbol_cleanup();
}
