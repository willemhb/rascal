#include <ctype.h>
#include <string.h>

#include "obj/stream.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
struct vtable_t StreamMethods =
  {
    NULL, NULL, NULL
  };

struct layout_t StreamLayout =
  {
    .vmtype=vmtype_stream
  };

struct datatype_t StreamType =
  {
    {
      .obj=obj_init(&TypeType, sizeof(datatype_t), object_fl_static),
      .name="stream"
    },

    .layout=&StreamLayout,
    .methods=&StreamMethods
  };

stream_t Ins, Outs, Errs;

/* API */
int peekc( stream_t stream )
{
  int out = fgetc(stream);

  if ( out != EOF )
    ungetc(out, stream);

  return out;
}

int skipws( stream_t stream )
{
  int ch;

  while ((ch=peekc(stream)) != EOF && isspace(ch))
    fgetc(stream);

  return ch;
}

int strskip( stream_t stream, const char *chars )
{
  int ch;

  while ((ch=peekc(stream)) != EOF && strchr(chars, ch)) fgetc(stream);

  return ch;
}

/* runtime */
void rl_obj_stream_init( void )
{
  Ins  = stdin;
  Outs = stdout;
  Errs = stderr;

  gl_init_type(StreamType);
}

void rl_obj_stream_mark( void )
{
  gl_mark_type(StreamType);
}

/* convenience */
