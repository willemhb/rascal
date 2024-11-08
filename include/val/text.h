#ifndef rl_val_text_h
#define rl_val_text_h

#include "val/object.h"

#include "util/text.h"
#include "util/table.h"

/* Types and APIs for Rascal values used in IO, binary data processing, and text representation, as well as supporting globals. */
// user IO types

struct Str {
  HEADER;
  // bit fields
  uint8  cached;

  // data fields
  hash64 chash;  // character hash (distinct from object hash)
  size64 cnt;
  char*  cs;
};

/* Globals */
/* APIs */
/* Str API */
#define is_str(x) has_type(x, T_STR)
#define as_str(x) ((Str*)as_obj(x))

Str*   new_str(char* cs, size64 n, bool i, hash64 h);
Str*   mk_str(char* cs, size64 n, bool i);
Str*   get_str(char* cs, size64 n);
Glyph  str_ref(Str* s, size64 n);
Str*   str_set(Str* s, size64 n, Glyph g);
size64 str_buf(Str* s, char* buf, size64 bufsz);

#endif
