#include "rl/readers/runtime.h"

/* runtime methods */
void rl_rl_readers_init( void )
{
  rl_rl_readers_atom_init();
  rl_rl_readers_list_init();
  rl_rl_readers_number_init();
  rl_rl_readers_space_init();
}

void rl_rl_readers_mark( void )
{
  rl_rl_readers_atom_mark();
  rl_rl_readers_list_mark();
  rl_rl_readers_number_mark();
  rl_rl_readers_space_mark();
}

void rl_rl_readers_cleanup( void )
{
  rl_rl_readers_atom_cleanup();
  rl_rl_readers_list_cleanup();
  rl_rl_readers_number_cleanup();
  rl_rl_readers_space_cleanup();
}
