#include <ctype.h>
#include <string.h>

#include "obj/stream.h"
#include "obj/type.h"

/* commentary */

/* C types */

/* globals */
struct type_t StreamType =
  {
    {
      .type=&TypeType.data,
      .size=sizeof(struct type_t)
    },

    {
      .name="stream",
      .vmtype=vmtype_object,
      .obsize=sizeof(stream_t),
      .elsize=0,
      .stringp=false
    }
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
}

/* convenience */
