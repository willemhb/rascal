#include <ctype.h>

#include "obj/stream.h"

#include "vm/error.h"
#include "vm/obj/reader.h"

#include "rl/read.h"

/* commentary */

/* C types */

/* globals */

/* API */
int read_expression( reader_t *reader, int dispatch );

value_t read( stream_t stream )
{
  reset_reader(&Reader, stream);
  read_expression(&Reader, 0);

  return take_value(&Reader);
}

value_t readln( stream_t stream )
{
  value_t out = read(stream);
  fprintf(stream, "\n");

  return out;
}

int read_expression( reader_t *reader, int dispatch )
{
  if ( dispatch )
    accumulate_character(reader, dispatch);

  while ( !reader->readstate )
    {
      int dispatch = peekc(reader->stream);
      reader_dispatch_fn_t reader_fn = get_dispatch_fn(reader, dispatch);

      if ( reader_fn == NULL )
	return panic("No dispatch function for %c", dispatch);

      reader_fn(reader, dispatch);
    }

  return 1;
}

/* runtime */
void rl_rl_read_init( void ) {}
void rl_rl_read_mark( void ) {}
void rl_rl_read_cleanup( void ) {}
