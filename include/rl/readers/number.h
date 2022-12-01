#ifndef rl_rl_readers_number_h
#define rl_rl_readers_number_h

#include "rascal.h"

/* commentary */

/* C types */

/* globals */

/* API */
void read_signed_number( reader_t *reader, int dispatch );
void read_unsigned_number( reader_t *reader, int dispatch );

/* runtime */
void rl_rl_readers_number_init( void );
void rl_rl_readers_number_mark( void );
void rl_rl_readers_number_cleanup( void );

#endif
