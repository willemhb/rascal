#include <stdarg.h>

#include "vm/error.h"
#include "vm/memory.h"
#include "vm/obj/reader.h"

#include "obj/stream.h"

/* commentary */

/* C types */

/* globals */
reader_t Reader;

/* API */
/* high level */
void clear_reader( reader_t *reader )
{
  reader->value = NUL;
  set_status(reader, readstate_ready, NULL);
  reset_ascii_buffer(reader->buffer);
}

void reset_reader( reader_t *reader, stream_t stream )
{
  clear_reader(reader);

  reader->stream = stream;
}


int give_value( reader_t *reader, value_t value )
{
  reader->value = value;
  return set_status( reader, readstate_expr, NULL );
}

value_t take_value( reader_t *reader )
{
  value_t out = reader->value;
  clear_reader(reader);
  return out;
}

extern int vpanic(const char *fmt, va_list va);

int set_status( reader_t *reader, readstate_t status, const char *fmt, ... )
{
  reader->readstate = status;

  int status_out=1;

  if ( fmt != NULL )
    {
      va_list va; va_start(va, fmt);

      if ( status == readstate_error )
	status_out = vpanic(fmt, va);

      else
	vfprintf(Outs, fmt, va);

      va_end(va);
    }

  return status_out;
}

/* buffer interface */
int accumulate_character( reader_t *reader, int character )
{
  ascii_buffer_push(reader->buffer, character);
  return reader->buffer->len;
}

/* stream interface */
bool is_eos( reader_t *reader )
{
  return feof(reader->stream);
}

/* readtable interface */
reader_dispatch_fn_t get_dispatch_fn( reader_t *reader, int character )
{
  return readtable_get(reader->readtable, character);
}

int set_reader_macro( reader_t *reader, int dispatch, reader_dispatch_fn_t handler )
{
  return readtable_add(reader->readtable, dispatch, handler);
}

int set_reader_macros( reader_t *reader, char *dispatches, reader_dispatch_fn_t handler )
{
  int out = 0;
  for (size_t i=0; dispatches[i] != '\0'; i++)
    out = set_reader_macro(reader, dispatches[i], handler);

  return out;
}

/* runtime */
void rl_vm_obj_reader_init( void )
{
  Reader.stream = Ins;
  Reader.buffer = make_ascii_buffer(0);
  Reader.readtable = make_readtable(256, NULL);
  Reader.readstate  = readstate_ready;
  Reader.value  = NUL;
}

void rl_vm_obj_reader_mark( void )
{
  mark_value(Reader.value);
}

/* convenience */
