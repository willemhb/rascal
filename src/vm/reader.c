#include "obj/buffer.h"
#include "obj/stream.h"

#include "vm/reader.h"
#include "vm/memory.h"

/* commentary */

/* C types */

/* globals */
reader_t Reader;

/* API */
void clear_reader( reader_t *reader )
{
  reader->token = token_ready;
  reader->value = NUL;

  clear_array(reader->buffer, buffer_len(reader->buffer), sizeof(char));
}

void reset_reader( reader_t *reader, stream_t stream )
{
  clear_reader(reader);

  reader->stream = stream;
}

/* runtime */
void rl_vm_reader_init( void )
{
  Reader.stream = Ins;
  Reader.buffer = make_buffer(0, NULL);
  Reader.token  = token_ready;
  Reader.value  = NUL;
}

/* convenience */
