#ifndef rl_obj_stream_h
#define rl_obj_stream_h

#include "vm/value.h"

/* commentary */

/* C types */

/* globals */
extern datatype_t StreamType;
extern stream_t Ins, Outs, Errs;

/* API */

/* runtime */
void rl_obj_stream_init( void );
void rl_obj_stream_mark( void );
void rl_obj_stream_cleanup( void );

/* convenience */
#define is_stream( x ) (((x)&TAGMASK)==STREAM)
#define as_stream( x ) ((stream_t)((x)&PTRMASK))

int  peekc( stream_t stream );
int  skipws( stream_t stream );
int  strskip( stream_t stream, const char *chars );
bool issep( int character ); // whitespace or ','
bool isdlm( int character ); // '()', '[]', or '{}'

static inline bool is_std_stream( stream_t x )
{
  return x == stdin || x == stdout || x == stderr;
}

#endif
