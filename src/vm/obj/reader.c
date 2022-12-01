#include <stdarg.h>

#include "vm/error.h"
#include "vm/memory.h"
#include "vm/obj/reader.h"

#include "obj/stream.h"

/* commentary */

/* C types */

/* globals */
reader_t Reader;

#define READER_NKEYS 256

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


void give_value( reader_t *reader, value_t value )
{
  reader->value = value;
  set_status( reader, readstate_expr, NULL );
}

value_t take_value( reader_t *reader )
{
  value_t out = reader->value;
  clear_reader(reader);
  return out;
}

extern void vpanic(const char *fmt, va_list va);

void set_status( reader_t *reader, readstate_t status, const char *fmt, ... )
{
  reader->readstate = status;

  if ( fmt != NULL )
    {
      va_list va; va_start(va, fmt);

      if ( status == readstate_error )
	vpanic(fmt, va);

      else
	vfprintf(Outs, fmt, va);

      va_end(va);
    }
}

/* buffer interface */
void accumulate_character( reader_t *reader, int character )
{
  ascii_buffer_push(reader->buffer, character);
}

/* stream interface */
bool is_eos( reader_t *reader )
{
  return feof(reader->stream);
}

/* readtable interface */
reader_fn_t get_dispatch_fn( reader_t *reader, int character )
{
  return readtable_get(reader->readtable, character);
}

void set_reader_macro( reader_t *reader, int dispatch, reader_fn_t handler )
{
  readtable_add(reader->readtable, dispatch, handler);
}

void set_reader_macros( reader_t *reader, char *dispatches, reader_fn_t handler )
{
  for (size_t i=0; dispatches[i] != '\0'; i++)
    set_reader_macro(reader, dispatches[i], handler);
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
