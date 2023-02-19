#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "types/text.h"
#include "types/table.h"

#include "runtime/memory.h"

#include "util/hash.h"
#include "util/collections.h"

/* globals */
Bin EmptyString;
Stream Stdin, Stdout, Stderr;
Table FlagsToMode;

/* API */
// stream api -----------------------------------------------------------------
void init_stream(Stream* stream, flags fl, FILE* ios) {
  init_obj((Obj*)stream, STREAM_TYPE);

  stream->obj.flags = fl;
  stream->ios       = ios;
}

Stream* new_stream(flags fl, FILE* ios) {
  Stream* out = construct(STREAM_TYPE, 1, 0);

  init_stream(out, fl, ios);

  return out;
}

Stream* sopen(flags fl, Val fname) {
  Val vmode = table_get(&FlagsToMode, tag(fl));

  assert(vmode != NOTFOUND);

  char* mode = as_text(vmode);
  char* name = as_text(fname);

  assert(name);

  FILE* ios = fopen(name, mode);

  assert(ios);

  return new_stream(fl, ios);
}

int sclose(Stream* stream) {
  return fclose(stream->ios);
}

bool seof(Stream* stream) {
  return feof(stream->ios);
}

int sputc(Stream* stream, int c) {
  return fputc(c, stream->ios);
}

int sgetc(Stream* stream) {
  return fgetc(stream->ios);
}

int sungetc(Stream* stream, int c) {
  if (!seof(stream))
    return ungetc(c, stream->ios);

  return EOF;
}

int speekc(Stream* stream) {
  int c = sgetc(stream);

  if (c != EOF)
    return sungetc(stream, c);

  return EOF;
}

int sfmt(Stream* stream, char* fmt, ...) {
  va_list va;

  va_start(va, fmt);
  int out = vsfmt(stream, fmt, va);
  va_end(va);
  return out;
}

int vsfmt(Stream* stream, char* fmt, va_list va) {
  return vfprintf(stream->ios, fmt, va);
}

// bin api --------------------------------------------------------------------
uhash hash_bin_data(uint n, void* data) {
  static uhash th = 0;

  if (th == 0)
    th = hash_uint(BIN_TYPE);

  uhash dh = hash_mem(data, n);

  return mix_hashes(2, dh, th);
}

void init_bin(Bin* self, bool encoded, uint n, void* data) {
  init_obj((Obj*)self, BIN_TYPE);
  
  self->obj.flags = ENCODED*encoded;
  self->count     = n;
  self->cap       = pad_alist_size(n+encoded, 0);
  self->array     = allocate(self->cap);

  if (data)
    memcpy(self->array, data, self->cap);

  if (data && encoded)
    self->hash = hash_bin_data(n, data);
}

Bin* new_bin(bool encoded, uint n, void* data) {
  Bin* out = construct(BIN_TYPE, 1, 0);

  init_bin(out, encoded, n, data);

  return out;
}

Bin* bytes(uint n, ubyte* bytes) {
  return new_bin(false, n, bytes);
}

Bin* string(char* chars) {
  return new_bin(true, strlen(chars), chars);
}

Bin* bytecode(uint n, uint16* code) {
  return new_bin(false, n*2, code);
}

bool is_string(Bin* bin) {
  return has_flag((Obj*)bin, ENCODED);
}

void resize_bin(Bin* self, uint n) {
  uint cap = pad_alist_size(n+is_string(self), self->cap);

  if (cap != self->cap) {
    self->array = reallocate(self->array, cap, self->cap);
    self->cap   = cap;
  }

  self->count = n;

  if (is_string(self))
    self->hash = 0;
}

void* bin_peep(Bin* self, int i) {
  if (i < 0)
    i += self->count;

  assert(i >= 0 && (uint)i < self->count);
  return self->array + i;
}

ubyte bin_ref(Bin* self, int i) {
  return *(ubyte*)bin_peep(self, i);
}

ubyte bin_set(Bin* self, int i, ubyte byte) {
  return (*(ubyte*)bin_peep(self, i) = byte);
}

usize bin_write(Bin* self, usize n, void* data) {
  usize off = self->count;
  
  resize_bin(self, self->count+n);

  memcpy(self->array+off, data, n);

  return self->count;
}

// initialization -------------------------------------------------------------
void text_init(void) {
  // initialize globals -------------------------------------------------------
  init_bin(&EmptyString, true, 0, "");
  init_stream(&Stdin, TEXT|INPUT, stdin);
  init_stream(&Stdout, TEXT|OUTPUT, stdout);
  init_stream(&Stderr, TEXT|OUTPUT, stderr);

  // initialize types ---------------------------------------------------------
  extern usize destruct_stream(void* self);
  extern void  print_stream(Val x, void* state);
  extern uhash hash_stream(Val x, void* state);
  extern bool  equal_streams(Val x, Val y, void* state);
  extern int   compare_streams(Val x, Val y, void* state);

  MetaTables[STREAM_TYPE] = (Mtable) {
    .name        = intern("stream"),
    .type        = STREAM_TYPE,
    .kind        = DATA_KIND,
    .type_hash   = hash_uint(STREAM_TYPE),
    .size        = sizeof(Stream),
    .destruct    = destruct_stream,
    .print       = print_stream,
    .hash        = hash_stream,
    .equal       = equal_streams,
    .compare     = compare_streams
  };

  extern usize destruct_bin(void* self);
  extern void  print_bin(Val x, void* state);
  extern uhash hash_bin(Val x, void* state);
  extern bool  equal_bins(Val x, Val y, void* state);
  extern int   compare_bins(Val x, Val y, void* state);

  MetaTables[BIN_TYPE] = (Mtable) {
    .name         = intern("bin"),
    .type         = BIN_TYPE,
    .kind         = DATA_KIND,
    .type_hash    = hash_uint(BIN_TYPE),
    .size         = sizeof(Bin),
    .destruct     = destruct_bin,
    .print        = print_bin,
    .hash         = hash_bin,
    .equal        = equal_bins,
    .compare      = compare_bins
  };

  extern int  write_glyph(Val x, Type type, int size, void* buf);
  extern void print_glyph(Val x, void* state);
  extern int  compare_glyphs(Val x, Val y, void* state);

  MetaTables[GLYPH_TYPE] = (Mtable) {
    .name         = intern("glyph"),
    .type         = GLYPH_TYPE,
    .kind         = DATA_KIND,
    .type_hash    = hash_uint(GLYPH_TYPE),
    .size         = sizeof(Glyph),
    .write        = write_glyph,
    .print        = print_glyph,
    .compare      = compare_glyphs
  };
  
  // initialize mode table ----------------------------------------------------
  init_table(&FlagsToMode, false, 0, NULL);

  table_set(&FlagsToMode, tag(INPUT), tag("r"));
  table_set(&FlagsToMode, tag(INPUT|TEXT), tag("rt"));
  table_set(&FlagsToMode, tag(INPUT|BINARY), tag("rb"));
  table_set(&FlagsToMode, tag(INPUT|OUTPUT), tag("r+"));
  table_set(&FlagsToMode, tag(INPUT|TEXT|OUTPUT), tag("rt+"));
  table_set(&FlagsToMode, tag(INPUT|BINARY|OUTPUT), tag("rb+"));

  table_set(&FlagsToMode, tag(OUTPUT), tag("w"));
  table_set(&FlagsToMode, tag(OUTPUT|TEXT), tag("wt"));
  table_set(&FlagsToMode, tag(OUTPUT|BINARY), tag("wb"));
  table_set(&FlagsToMode, tag(OUTPUT|CREATE), tag("w+"));
  table_set(&FlagsToMode, tag(OUTPUT|CREATE|TEXT), tag("wt+"));
  table_set(&FlagsToMode, tag(OUTPUT|CREATE|BINARY), tag("wb+"));
  
  table_set(&FlagsToMode, tag(OUTPUT|UPDATE), tag("a"));
  table_set(&FlagsToMode, tag(OUTPUT|UPDATE|TEXT), tag("at"));
  table_set(&FlagsToMode, tag(OUTPUT|UPDATE|BINARY), tag("ab"));
  table_set(&FlagsToMode, tag(OUTPUT|UPDATE|CREATE), tag("a+"));
  table_set(&FlagsToMode, tag(OUTPUT|UPDATE|CREATE|TEXT), tag("at+"));
  table_set(&FlagsToMode, tag(OUTPUT|UPDATE|CREATE|BINARY), tag("ab+"));
}
