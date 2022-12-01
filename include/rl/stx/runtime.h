#ifndef rl_rl_stx_h
#define rl_rl_stx_h

#include "rl/stx/quote.h"
#include "rl/stx/if.h"
#include "rl/stx/fun.h"
#include "rl/stx/do.h"
#include "rl/stx/var.h"

/* runtime dispatch module for stx subdirectory */

/* runtime */
void rl_rl_stx_init( void );
void rl_rl_stx_mark( void );
void rl_rl_stx_cleanup( void );

#endif
