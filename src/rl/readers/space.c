#include <ctype.h>

#include "vm/obj/reader.h"

#include "obj/stream.h"

#include "rl/readers/space.h"

#include "util/string.h"

/* commentary 

   reader macros for terminal and separator characters.

   nb: ',' is considered a whitespace character in early
   versions of rascal. */

/* C types */

/* globals */

/* API */
void read_space( reader_t *reader, int dispatch )
{
  while ( isspace(dispatch) )
      dispatch = fgetc(reader->stream);
      
  ungetc(dispatch, reader->stream);
}

void read_comment( reader_t *reader, int dispatch )
{
  while ( dispatch != '\n' )
      dispatch = fgetc(reader->stream);

  dispatch = fgetc(reader->stream);

  if (dispatch != EOF)
    ungetc(dispatch, reader->stream);
}

void read_eos( reader_t *reader, int dispatch )
{
  (void)dispatch;

  reader->readstate = readstate_eos;
}

/* runtime */
void rl_rl_readers_space_init( void )
{
  /* whitespace and separators */
  set_reader_macro(&Reader, '\n', read_space);
  set_reader_macro(&Reader, '\v', read_space);
  set_reader_macro(&Reader, '\f', read_space);
  set_reader_macro(&Reader, '\r', read_space);
  set_reader_macro(&Reader, ' ', read_space);
  set_reader_macro(&Reader, ',', read_space);

  set_reader_macro(&Reader, ';', read_comment);

  set_reader_macro(&Reader, EOF, read_eos);
}

void rl_rl_readers_space_mark( void ) {}

void rl_rl_readers_space_cleanup( void ) {}
