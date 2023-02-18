#ifndef stream_h
#define stream_h

#include <stdio.h>

#include "base/object.h"

#include "util/ctype.h"

/* C types */
typedef enum StreamFl StreamFl;

enum StreamFl {
  INSTREAM =0b00000001,
  OUTSTREAM=0b00000010,
  BINSTREAM=0b00000100,
  TXTSTREAM=0b00001000
};

struct Stream {
  Obj   obj;
  FILE* ios;
  Ctype encoding;
};

/* globals */
extern Stream Ins, Outs, Errs;

/* API */
bool    is_stream(Val x);
Stream* as_stream(Val x);
Val     mk_stream(Stream *p);

// convenience ----------------------------------------------------------------
bool peof(Stream* stream);
int  pputc(Stream* stream, int c);
int  pgetc(Stream* stream);
int  sungetc(Stream* stream, int c);
int  speekc(Stream* stream);
int  sclose(Stream* stream);
int  sfmt(Stream* stream, char* fmt, ...);

#endif
