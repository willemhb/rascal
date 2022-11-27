#ifndef rl_obj_stream_h
#define rl_obj_stream_h

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */
extern struct type_t StreamType;
extern stream_t Ins, Outs, Errs;

/* API */
int peekc( stream_t stream );
int skipws( stream_t stream );
int strskip( stream_t stream, const char *chars );

/* runtime */
void rl_obj_stream_init( void );

/* convenience */
#define is_stream( x ) (((x)&TAGMASK)==STREAM)
#define as_stream( x ) ((stream_t)((x)&PTRMASK))

static inline bool is_std_stream( stream_t x )
{
  return x == stdin || x == stdout || x == stderr;
}

#endif
