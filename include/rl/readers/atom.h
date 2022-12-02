#ifndef rl_rl_readers_atom_h
#define rl_rl_readers_atom_h

#include "rascal.h"

/* commentary

   Mostly symbols, but includes other types like bool() and nul(). */

/* C types */

/* globals */

/* API */
int read_atom( reader_t *reader, int dispatch );

/* runtime */
void rl_rl_readers_atom_init( void );
void rl_rl_readers_atom_mark( void );
void rl_rl_readers_atom_cleanup( void );

#endif
