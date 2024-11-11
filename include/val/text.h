#ifndef rl_val_text_h
#define rl_val_text_h

#include "val/object.h"

#include "util/text.h"
#include "util/table.h"

/* Types and APIs for Rascal values used in IO, binary data processing, and text representation, as well as supporting globals. */
// user IO types
// rascal FILE wrapper type
struct Port {
  HEADER;

  bool  isrl; // represents Rascal source (versus plain text and binary)
  FILE* ios;  // wrapped FILE object
};

// rascal string type
struct Str {
  HEADER;
  // bit fields
  uint8  cached; // whether the string is interned in the global string table

  // data fields
  hash64 chash;  // character hash (distinct from object hash)
  size64 cnt;    // doesn't include terminal nul byte (so actual size is cnt+1)
  char*  cs;     // character array
};

struct Buffer {
  HEADER;
  // bit fields
  CType  type;
  size8  elsize;
  flags8 encoded;

  // data fields
  size64 cnt, cap;
  byte*  data;
};

/* Globals */
/* External APIs */
// Port API
#define is_port(x) has_type(x, T_PORT)
#define as_port(x) ((Port*)as_obj(x))

Port* rl_popen(Str* fname, Str* fmode);
int   rl_pclose(Port* p);
bool  rl_pisopen(Port* p);
bool  rl_pisclosed(Port* p);

// Str API
#define is_str(x) has_type(x, T_STR)
#define as_str(x) ((Str*)as_obj(x))

Str*    new_str(char* cs, size64 n, bool i, hash64 h);
Str*    mk_str(char* cs, size64 n, bool i);
Str*    get_str(char* cs, size64 n);
Glyph   str_ref(Str* s, size64 n);
Str*    str_set(Str* s, size64 n, Glyph g);
bool    egal_str_obs(Str* sx, Str* sy);
int     order_str_obs(Str* sx, Str* sy);
size64  str_buf(Str* s, char* buf, size64 bufsz);

// Buffer API
#define is_buffer(x) has_type(x, T_BUFFER)
#define as_buffer(x) ((Buffer*)as_obj(x))

// mostly internal methods
Buffer* new_buffer(CType type);
void    init_buffer(State* vm, Buffer* b, CType type);
void    shrink_buffer(Buffer* b, size64 n);
void    grow_buffer(Buffer* b, size64 n);
void    resize_buffer(Buffer* b, size64 n);

// external methods
void*  buffer_ref(Buffer* b, size64 n);
size64 buffer_add(Buffer* b, word_t d);
size64 buffer_wrt(Buffer* b, size64 n, byte* d);
void   buffer_set(Buffer* b, size64 n, word_t d);

#endif
