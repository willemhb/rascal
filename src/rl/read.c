#include <ctype.h>

#include "obj/stream.h"

#include "vm/obj/reader.h"

#include "rl/read.h"

/* commentary */

/* C types */

/* globals */

/* API */
static int nextc( reader_t *reader )
{
  stream_t stream = reader->stream;
  int ch = skipws(stream);
  
  if ( ch == EOF )
    reader->token = token_eos;

  return ch;
}

value_t read( stream_t stream )
{
  reset_reader(&Reader, stream);

  int ch = nextc(&Reader);
}

/* runtime */
void rl_rl_read_init( void ) {}
void rl_rl_read_mark( void ) {}
