#ifndef stream_h
#define stream_h

#include "base/object.h"

/* C types */
typedef enum StreamFl StreamFl;

enum StreamFl {
  ASCII_STREAM =0x01,
  BIN8_STREAM  =0x02,
  INPUT_STREAM =0x08,
  OUTPUT_STREAM=0x10
};

struct Stream {
  Obj obj;
  FILE* ios;
};

/* globals */
extern Stream Ins, Outs, Errs;

/* API */
bool  is_stream(Val x);
Stream* as_stream(Val x);
Val   tag_stream(Stream *p);
Stream *mk_stream(FILE *ios);

bool peof(Stream* stream);
int  pputc(Stream* stream, int c);
int  pgetc(Stream* stream);
int  sungetc(Stream* stream, int c);
int  speekc(Stream* stream);
int  sclose(Stream* stream);
int  sfmt(Stream* stream, char* fmt, ...);

#endif
