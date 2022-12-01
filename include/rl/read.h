#ifndef rl_rl_read_h
#define rl_rl_read_h

#include "rascal.h"

#include "vm/obj/reader.h"

/* commentary */

/* C types */
/* signature for procedures that go in the readtable */

/* globals */

/* API */
value_t readln( stream_t stream );
value_t read( stream_t stream );

/* runtime */
void rl_rl_read_init( void );
void rl_rl_read_mark( void );
void rl_rl_read_cleanup( void );

/* convenience */

#endif
