#include "rl/runtime.h"

/* runtime dispatch */
void rl_rl_init( void )
{
  rl_rl_compile_init();
  rl_rl_eval_init();
  rl_rl_apply_init();
  rl_rl_exec_init();
  rl_rl_prin_init();
  rl_rl_read_init();
  rl_rl_repl_init();

  rl_rl_readers_init();
  rl_rl_native_init();
  rl_rl_stx_init();
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

  rl_rl_readers_mark();
  rl_rl_native_mark();
  rl_rl_stx_mark();
}

void rl_rl_cleanup( void )
{
  rl_rl_compile_cleanup();
  rl_rl_eval_cleanup();
  rl_rl_apply_cleanup();
  rl_rl_exec_cleanup();
  rl_rl_prin_cleanup();
  rl_rl_read_cleanup();
  rl_rl_repl_cleanup();

  rl_rl_readers_cleanup();
  rl_rl_native_cleanup();
  rl_rl_stx_cleanup();
}
