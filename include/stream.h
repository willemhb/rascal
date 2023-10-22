#ifndef rascal_stream_h
#define rascal_stream_h

#include "object.h"

/* reader, io types */
// C types
typedef enum {
  READ_MODE     = 0x000,
  CREATE_MODE   = 0x001,
  UPDATE_MODE   = 0x002,
  WRITE_MODE    = 0x003,
  TEXT_STREAM   = 0x004,
  BINARY_STREAM = 0x008,
  INPUT_STREAM  = 0x010,
  OUTPUT_STREAM = 0x020,
} StrmFl;

struct Stream {
  Obj      obj;
  FILE*    ios;
  Encoding enc;
};

// globals
extern struct Type StreamType, GlyphType;
extern struct Stream Ins, Outs, Errs;
extern struct Table ModesToFlags;

// external API
// constructors
Stream* new_stream(FILE* ios, Encoding enc, flags_t mode);

// stream API (largely wraps C file API)
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

// 

// tooplevel initialization
void init_std_streams(void);

#endif
