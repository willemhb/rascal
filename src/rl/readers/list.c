
#include "vm/error.h"
#include "vm/value.h"
#include "vm/memory.h"

#include "vm/obj/reader.h"
#include "vm/obj/heap.h"

#include "obj/cons.h"
#include "obj/stream.h"

#include "rl/read.h"
#include "rl/readers/list.h"

/* commentary */

/* C types */

/* globals */

/* API */
/* internal read procedure; exposed here so that reader and dispatch may be passed. */
extern int read_expression( reader_t *reader, int dispatch );

int read_list( reader_t *reader, int dispatch )
{
  int status; value_t out = NUL, last = NUL, old_last, value;

  save_values(1, &out);
  
  while ((dispatch=fgetc(reader->stream)) != EOF && dispatch != ')' && dispatch != '.')
    {
      if ( (status=read_expression(reader, dispatch)) < 0 )
	return status;

      value = take_value(reader);

      if ( last == NUL )
	last = out = make_cons(value, NUL);

      else
	{
	  old_last = last;
	  last     = make_cons(value, NUL);

	  set_cons_cdr(as_cons(old_last), last);
	}
    }

  if ( dispatch == EOF )
      return set_status(reader, readstate_error, "Unexpected EOS reading list.");

  else if ( dispatch == '.' )
    {
      read_expression(reader, 0);

      if ( (dispatch=skipws(reader->stream)) == EOF )
	  return set_status(reader, readstate_error, "Unexpected EOS reading dotted list.");

      else if ( dispatch != ')' )
	return set_status(reader, readstate_error, "Unexpected token reading dotted list.");

      else
	set_cons_cdr(as_cons(last), take_value(reader));
    }

  else if ( dispatch != ')' )
    return set_status(reader, readstate_error, "Unexpected token reading dotted list.");

  return give_value(reader, out); // build list
}

int read_list_rpar_error( reader_t *reader, int dispatch )
{
  (void)dispatch;

  return set_status(reader, readstate_error, "unexpected ')'.\n");
}

int read_list_dot_error( reader_t *reader, int dispatch )
{
  (void)dispatch;

  return set_status(reader, readstate_error, "unexpected '.'.\n");
}

/* runtime */
void rl_rl_readers_list_init( void )
{
  set_reader_macro(&Reader, '(', read_list);
  set_reader_macro(&Reader, ')', read_list_rpar_error);
  set_reader_macro(&Reader, '.', read_list_dot_error);
}

void rl_rl_readers_list_mark( void ) {}

void rl_rl_readers_list_cleanup( void ) {}
