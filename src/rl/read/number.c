#include <ctype.h>

#include "vm/obj/reader.h"

#include "obj/stream.h"

#include "rl/read/number.h"

/* commentary */

/* C types */

/* globals */

/* API */
void read_signed_number( reader_t *reader, int dispatch )
{
  accumulate_character(reader, dispatch);

  
}

void read_unsigned_number( reader_t *reader, int dispatch )
{
  
}

/* runtime */
void rl_rl_read_number_init( void )
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

void rl_rl_read_number_mark( void ) {}

