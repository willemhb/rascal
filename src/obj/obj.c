#include "obj/obj.h"

/* runtime */
void rl_obj_init( void )
{
  rl_obj_control_init();
  rl_obj_lambda_init();
  rl_obj_nul_init();
  rl_obj_real_init();
  rl_obj_stream_init();
  rl_obj_type_init();
  rl_obj_cons_init();
  rl_obj_symbol_init();
  rl_obj_native_init();
  rl_obj_bool_init();
}

void rl_obj_mark( void )
{
  rl_obj_control_mark();
  rl_obj_lambda_mark();
  rl_obj_nul_mark();
  rl_obj_real_mark();
  rl_obj_stream_mark();
  rl_obj_type_mark();
  rl_obj_cons_mark();
  rl_obj_symbol_mark();
  rl_obj_native_mark();
  rl_obj_bool_mark();
}
