#include <wctype.h>

#include "runtime.h"

#include "environment.h"
#include "type.h"
#include "stream.h"

// globals
struct Type StreamType, GlyphType;

Stream Ins = {
  .obj={
    .type    =&StreamType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true
  }
};

Stream Outs = {
  .obj={
    .type    =&StreamType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true
  }
};

Stream Errs = {
  .obj={
    .type    =&StreamType,
    .annot   =&EmptyMap,
    .no_sweep=true,
    .no_free =true,
    .gray    =true
  }
};

// external API
// constructors
Stream* new_stream(FILE* ios);

// 
Stream* sopen(Obj* path, const char* fname, const char* opts);
int     sclose(Stream* stream);
bool    seof(Stream* stream);
Glyph   sget(Stream* stream);
Glyph   sunget(Stream* stream, Glyph g);
Glyph   speek(Stream* stream);
Glyph   sput(Stream* stream, Glyph g);
String* sread(Stream* stream);
int     swrite(Stream* stream, String* str);
int     spf(Stream* stream, const char* fmt, ...);

// character API
bool  isgalnum(Glyph g);
bool  isgalpha(Glyph g);
bool  isglower(Glyph g);
bool  isgupper(Glyph g);
bool  isgdigit(Glyph g);
bool  isgxdigit(Glyph g);
Glyph toglower(Glyph g);
Glyph togupper(Glyph g);

// tooplevel initialization
void init_std_streams(void) {
  // 
  Ins.ios  = stdin;
  Outs.ios = stdout;
  Errs.ios = stderr;

  // create toplevel bindings
  define(NULL, symbol("&ins"),  tag(&Ins),  CONSTANT);
  define(NULL, symbol("&outs"), tag(&Outs), CONSTANT);
  define(NULL, symbol("&errs"), tag(&Errs), CONSTANT);
}
