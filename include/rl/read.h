#ifndef rl_rl_read_h
#define rl_rl_read_h

#include "rascal.h"

#include "vm/obj/reader.h"

/* commentary */

/* C types */
/* signature for procedures that go in the readtable */
typedef void (*reader_dispatch_t)(reader_t *reader, int dispatch);

/* globals */

/* API */
value_t read( stream_t stream );

/* runtime */
void rl_rl_read_init( void );
void rl_rl_read_mark( void );

/* convenience */

#endif
