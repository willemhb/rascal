#ifndef rl_rl_readers_space_h
#define rl_rl_readers_space_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
void read_space( reader_t *reader, int dispatch );
void read_comment( reader_t *reader, int dispatch );
void read_eos( reader_t *reader, int dispatch );

/* runtime */
void rl_rl_readers_space_init( void );
void rl_rl_readers_space_mark( void );
void rl_rl_readers_space_cleanup( void );

#endif
