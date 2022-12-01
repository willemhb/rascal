#include "rl/native/runtime.h"


/* runtime dispatch */
void rl_rl_native_init( void )
{
  rl_rl_native_arithmetic_init();
}

void rl_rl_native_mark( void )
{
  rl_rl_native_arithmetic_mark();
}

void rl_rl_native_cleanup( void )
{
  rl_rl_native_arithmetic_cleanup();
}
