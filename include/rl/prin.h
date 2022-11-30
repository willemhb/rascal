#ifndef rl_rl_prin_h
#define rl_rl_prin_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
void prin( stream_t stream, value_t x );
void prinln( stream_t stream, value_t x );

/* convenience */
void rl_rl_prin_init( void );
void rl_rl_prin_mark( void );

#endif
