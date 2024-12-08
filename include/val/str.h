#ifndef rl_val_str_h
#define rl_val_str_h

#include "val/object.h"

#include "util/text.h"
#include "util/table.h"

/* Types and APIs for Rascal values used in IO, binary data processing, and text representation, as well as supporting globals. */
// user IO types
// rascal FILE wrapper type
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

/* Globals */
/* API */
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

/* Initialization */

#endif
