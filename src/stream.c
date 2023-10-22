#include <wctype.h>

#include "runtime.h"

#include "environment.h"
#include "type.h"
#include "table.h"
#include "stream.h"

// globals
struct Type StreamType, GlyphType;

Stream Ins = {
  .obj={
    .type    =&StreamType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
    .flags   =INPUT_STREAM | TEXT_STREAM | READ_MODE,
  },
  .enc=ASCII,
};

Stream Outs = {
  .obj={
    .type    =&StreamType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
    .flags   =OUTPUT_STREAM | TEXT_STREAM | WRITE_MODE,
  },
  .enc=ASCII,
};

Stream Errs = {
  .obj={
    .type    =&StreamType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true,
    .flags   =OUTPUT_STREAM | TEXT_STREAM | WRITE_MODE,
  },
  .enc=ASCII,
};

// external API
// constructors
Stream* new_stream(FILE* ios, Encoding enc, flags_t mode) {
  assert(ios != NULL);

  Stream* out = new_obj(&StreamType, mode, 0);

  out->ios = ios;
  out->enc = enc;

  return out;
}

// 

Stream* sopen(Obj* path, const char* fname, const char* opts) {
  
}

int sclose(Stream* stream) {
  assert(stream != NULL);
  require(stream->ios, "close", "close called twice on same `Stream`");
  fclose(stream->ios);
  
  stream->ios = NULL;

  return 0;
}

bool seof(Stream* stream) {
  assert(stream != NULL);
  assert(stream->ios != NULL);

  return feof(stream->ios);
}

Glyph sget(Stream* stream) {
  assert(stream != NULL);
  assert(stream->ios != NULL);

  return fgetc(stream->ios);
}

Glyph sunget(Stream* stream, Glyph g) {
  assert(stream != NULL);
  assert(stream->ios != NULL);

  return ungetc(g, stream->ios);
}

Glyph speek(Stream* stream) {
  assert(stream != NULL);
  assert(stream->ios != NULL);

  int out = fgetc(stream->ios);

  if (out != EOF)
    ungetc(out, stream->ios);

  return out;
}

Glyph sput(Stream* stream, Glyph g) {
  assert(stream != NULL);
  assert(stream->ios != NULL);

  return fputc(g, stream->ios);
}

String* sread(Stream* stream);
int     swrite(Stream* stream, String* str);
int     spf(Stream* stream, const char* fmt, ...);

// character API
bool isgalnum(Glyph g) {
  return iswalnum(g);
}

bool isgalpha(Glyph g) {
  return iswalnum(g);
}

bool isglower(Glyph g) {
  return iswalnum(g);
}

bool isgupper(Glyph g) {
  return iswalnum(g);
}

bool isgdigit(Glyph g) {
  return iswalnum(g);
}

bool isgxdigit(Glyph g) {
  return iswxdigit(g);
}

Glyph toglower(Glyph g) {
  return towlower(g);
}

Glyph togupper(Glyph g) {
  return towupper(g);
}

// tooplevel initialization
void init_std_streams(void) {
  // initialize ios pointers (not constant, can't be initialized statically)
  Ins.ios  = stdin;
  Outs.ios = stdout;
  Errs.ios = stderr;

  // create toplevel bindings
  define(NULL, symbol("&ins"),  tag(&Ins),  FINAL);
  define(NULL, symbol("&outs"), tag(&Outs), FINAL);
  define(NULL, symbol("&errs"), tag(&Errs), FINAL);

  // initialize modes-to-strings table
  
}
