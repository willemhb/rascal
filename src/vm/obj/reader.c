#include "obj/stream.h"

#include "vm/obj/reader.h"
#include "vm/memory.h"

/* commentary */

/* C types */

/* globals */
reader_t Reader;

#define READER_NKEYS 256

/* API */
void clear_reader( reader_t *reader )
{
  reader->readstate = readstate_ready;
  reader->value = NUL;

  reset_ascii_buffer(reader->buffer);
}

void reset_reader( reader_t *reader, stream_t stream )
{
  clear_reader(reader);

  reader->stream = stream;
}

void set_reader_macro( reader_t *reader, int dispatch, reader_fn_t handler )
{
  readtable_add(reader->readtable, dispatch, handler);
}

int peek_character( reader_t *reader )
{
  return peekc(reader->stream);
}

void accumulate_character( reader_t *reader, int character )
{
  ascii_buffer_push(reader->buffer, character);
}

/* runtime */
void rl_vm_reader_init( void )
{
  Reader.stream = Ins;
  Reader.buffer = make_ascii_buffer(0);
  Reader.readtable = make_readtable(256, NULL);
  Reader.readstate  = readstate_ready;
  Reader.value  = NUL;
}

void rl_vm_reader_mark( void )
{
  mark_value(Reader.value);
}

/* convenience */
