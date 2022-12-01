#ifndef rl_rl_readers_h
#define rl_rl_readers_h

#include "rl/readers/list.h"
#include "rl/readers/atom.h"
#include "rl/readers/space.h"
#include "rl/readers/number.h"

/* runtime dispatch module for the readers subdirectory */

/* runtime dispatch */
void rl_rl_readers_init( void );
void rl_rl_readers_mark( void );
void rl_rl_readers_cleanup( void );

#endif
