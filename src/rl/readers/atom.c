
#include "vm/obj/reader.h"

#include "obj/stream.h"
#include "obj/symbol.h"

#include "rl/readers/atom.h"

#include "util/string.h"

/* commentary */

/* C types */

/* globals */

/* API */
void read_atom( reader_t *reader, int dispatch )
{
  if ( dispatch != '\0' ) // sentinel for no accumulate
    accumulate_character(reader, dispatch);

  while ((dispatch=fgetc(reader->stream)) != EOF && !issep(dispatch) && !isdlm(dispatch) )
    accumulate_character(reader, dispatch);

  char *name = reader->buffer->elements;

  if ( streq(name, "true") )
    give_value(reader, TRUE);

  else if ( streq(name, "false") )
    give_value(reader, FALSE);

  else if ( streq(name, "nul") )
    give_value(reader, NUL);

  else
    give_value(reader, symbol(name));
}

/* runtime */
void rl_rl_read_atom_init( void )
{
  /* alphabetic characters */
  set_reader_macros(&Reader, "abcdefghijklmnopqrstuvwxyz", read_atom);
  set_reader_macros(&Reader, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", read_atom);

  /* fucked up characters */
  set_reader_macros(&Reader, ":<=>?@", read_atom);
  set_reader_macros(&Reader, "!$%&*/", read_atom);
  set_reader_macros(&Reader, "\\^_|", read_atom);
}

void rl_rl_readers_atom_mark( void ) {}
void rl_rl_readers_atom_cleanup( void ) {}
