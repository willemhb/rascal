#include "rl/rl.h"

/* runtime */
void rl_rl_init( void )
{
  rl_rl_compile_init();
  rl_rl_eval_init();
  rl_rl_apply_init();
  rl_rl_exec_init();
  rl_rl_prin_init();
  rl_rl_read_init();
  rl_rl_repl_init();

  rl_rl_read_list_init();
  rl_rl_read_atom_init();
  rl_rl_read_number_init();
  rl_rl_read_space_init();

  rl_rl_native_arithmetic_init();
}

void rl_rl_mark( void )
{
  rl_rl_compile_mark();
  rl_rl_eval_mark();
  rl_rl_apply_mark();
  rl_rl_exec_mark();
  rl_rl_prin_mark();
  rl_rl_read_mark();
  rl_rl_repl_mark();

  rl_rl_read_list_mark();
  rl_rl_read_atom_mark();
  rl_rl_read_number_mark();
  rl_rl_read_space_mark();

  rl_rl_native_arithmetic_init();
}
