#ifndef rl_obj_stream_h
#define rl_obj_stream_h

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */
extern type_t StreamType;
extern stream_t *Ins, *Outs, *Errs;

/* runtime */
void rl_obj_stream_init( void );
void rl_obj_stream_mark( void );

/* convenience */
static inline bool      is_stream( value_t x ) { return get_tag(x) == STREAM; }
static inline stream_t *as_stream( value_t x ) { return (stream_t*)as_pointer(x); }

static inline bool      is_std_stream( stream_t *x )
{
  return x == stdin || x == stdout || x == stderr;
}

#endif
