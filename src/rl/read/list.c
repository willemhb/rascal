
#include "vm/error.h"
#include "vm/value.h"

#include "vm/obj/reader.h"
#include "vm/obj/heap.h"

#include "obj/cons.h"
#include "obj/stream.h"

#include "rl/read.h"
#include "rl/read/list.h"

/* commentary */

/* C types */

/* globals */

/* API */
/* internal read procedure; exposed here so that reader and dispatch may be passed. */
extern void read_expression( reader_t *reader, int dispatch );

void read_list( reader_t *reader, int dispatch )
{
  size_t n = 0;
  
  while ((dispatch=fgetc(reader->stream)) != EOF && dispatch != ')' && dispatch != '.')
    {
      read_expression(reader, dispatch);
      save_vals(1, take_value(reader));
      n++;
    }

  if ( dispatch == EOF )
    {
      set_status(reader, readstate_error, "Unexpected EOS reading list.");
      return;
    }

  else if ( dispatch == '.' )
    {
      read_expression(reader, dispatch);

      if ( is_eos(reader) )
	{
	  set_status(reader, readstate_error, "Unexpected EOS reading dotted list.");
	  return;
	}

      save_vals(1, take_value(reader));

      if ( (dispatch=skipws(reader->stream)) == EOF )
	{
	  set_status(reader, readstate_error, "Unexpected EOS reading dotted list.");
	  return;
	}

      else if ( dispatch != ')' )
	{
	  set_status(reader, readstate_error, "Unexpected token reading dotted list.");
	  return;
	}
    }

  else
    save_vals(1, NUL); // list tail

  fgetc(reader->stream); // clear ')'
  give_value(reader, cons_v((value_t*)Heap.preserve_vals-n, n)); // build list
  unsave_vals(n);
}

void read_list_rpar_error( reader_t *reader, int dispatch )
{
  (void)dispatch;

  set_status(reader, readstate_error, "unexpected ')'.\n");
}

void read_list_dot_error( reader_t *reader, int dispatch )
{
  (void)dispatch;

  set_status(reader, readstate_error, "unexpected '.'.\n");
}

/* runtime */
void rl_rl_read_list_init( void )
{
  set_reader_macro(&Reader, '(', read_list);
  set_reader_macro(&Reader, ')', read_list_rpar_error);
  set_reader_macro(&Reader, '.', read_list_dot_error);
}

void rl_rl_read_list_mark( void ) {}

