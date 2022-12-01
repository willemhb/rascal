#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

#include "vm/obj/reader.h"

#include "obj/stream.h"
#include "obj/real.h"

#include "rl/readers/atom.h"
#include "rl/readers/number.h"

/* commentary */

/* C types */

/* globals */

/* API */
void read_signed_number( reader_t *reader, int dispatch )
{
  accumulate_character(reader, dispatch);

  bool dot = false;

  while ( !issep(dispatch=fgetc(reader->stream)) )
    {
      switch (dispatch)
	{
	case '0' ... '9':
	  accumulate_character(reader, dispatch);
	  break;

	case '.':
	  if ( !dot )
	    {
	      dot = true;
	      accumulate_character(reader, dispatch);
	      break;
	    }

	  rl_attr(fallthrough);

	default:
	  read_atom(reader, dispatch);
	  return;
	}
    }

  if ( ascii_buffer_ref(reader->buffer, -1) == '.' || reader->buffer->len == 1 )
    {
      read_atom(reader, 0);
      return;
    }

  char *buf;
  real_t num = strtod((const char *restrict)reader->buffer->elements, &buf);

  assert(*buf == '\0');
  give_value( reader, tag_real(num));
}

void read_unsigned_number( reader_t *reader, int dispatch )
{
  accumulate_character(reader, dispatch);

  bool dot = false;

  while ( !issep(dispatch=fgetc(reader->stream)) )
    {
      switch (dispatch)
	{
	case '0' ... '9':
	  accumulate_character(reader, dispatch);
	  break;

	case '.':
	  if ( !dot )
	    {
	      dot = true;
	      accumulate_character(reader, dispatch);
	      break;
	    }

	  rl_attr(fallthrough);

	default:
	  read_atom(reader, dispatch);
	  return;
	}
    }

  if ( ascii_buffer_ref(reader->buffer, -1) == '.' )
    {
      read_atom(reader, 0);
      return;
    }

  char *buf;
  real_t num = strtod((const char *restrict)reader->buffer->elements, &buf);

  assert(*buf == '\0');
  give_value( reader, tag_real(num));  
}

/* runtime */
void rl_rl_readers_number_init( void )
{
  set_reader_macro(&Reader, '+', read_signed_number);
  set_reader_macro(&Reader, '-', read_signed_number);

  set_reader_macro(&Reader, '1', read_unsigned_number);
  set_reader_macro(&Reader, '2', read_unsigned_number);
  set_reader_macro(&Reader, '3', read_unsigned_number);
  set_reader_macro(&Reader, '4', read_unsigned_number);
  set_reader_macro(&Reader, '5', read_unsigned_number);
  set_reader_macro(&Reader, '6', read_unsigned_number);
  set_reader_macro(&Reader, '7', read_unsigned_number);
  set_reader_macro(&Reader, '8', read_unsigned_number);
  set_reader_macro(&Reader, '9', read_unsigned_number);
  set_reader_macro(&Reader, '0', read_unsigned_number);
}

void rl_rl_readers_number_mark( void ) {}

void rl_rl_readers_number_cleanup( void ) {}
