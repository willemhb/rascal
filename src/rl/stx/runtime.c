#include "rl/stx/runtime.h"

/* runtime dispatch */
void rl_rl_stx_init( void )
{
  rl_rl_stx_quote_init();
  rl_rl_stx_if_init();
  rl_rl_stx_fun_init();
  rl_rl_stx_do_init();
  rl_rl_stx_val_init();
  rl_rl_stx_def_init();
}

void rl_rl_stx_mark( void )
{
  rl_rl_stx_quote_mark();
  rl_rl_stx_if_mark();
  rl_rl_stx_fun_mark();
  rl_rl_stx_do_mark();
  rl_rl_stx_val_mark();
  rl_rl_stx_def_mark();
}

void rl_rl_stx_cleanup( void )
{
  rl_rl_stx_quote_cleanup();
  rl_rl_stx_if_cleanup();
  rl_rl_stx_fun_cleanup();
  rl_rl_stx_do_cleanup();
  rl_rl_stx_val_cleanup();
  rl_rl_stx_def_cleanup();
}
