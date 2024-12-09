#ifndef rl_val_str_h
#define rl_val_str_h

#include "val/object.h"

#include "util/text.h"

/* Mutable and immutable encoded binary types. */
/* C types */
struct Str {
  HEADER;

  CType  encoding; // always ASCII atm
  bool   cached;   // whether the string is interned in the global string table
  hash64 chash;    // character hash (distinct from object hash)
  size64 cnt;      // doesn't include terminal nul byte (so actual size is cnt+1)
  char*  chars;    // character array
};

struct MStr {
  HEADER;
  CType  encoding;
  size64 cnt, max;
  char*  chars;
};

/* Globals */
extern Type StrType, MStrType;

/* APIs */
#define is_str(x) has_vtype(x, T_STR)
#define as_str(x) ((Str*)as_obj(x))

Str*    mk_str(char* cs, size64 n, bool i);
Glyph   str_ref(Str* s, size64 n);
Str*    str_set(Str* s, size64 n, Glyph g);
Str*    str_add(Str* s, Glyph g);
size64  str_cpy(Str* s, char* cs, size64 n);

#define is_mstr(x) has_vtype(x, T_MSTR)
#define as_mstr(x) ((MStr*)as_obj(x))

MStr*  new_mstr(char* cs, size64 n, bool i, hash64 h);
Glyph  mstr_ref(MStr* s, size64 n);
void   mstr_set(MStr* s, size64 n, Glyph g);
void   mstr_add(MStr* s, Glyph g);
void   mstr_write(MStr* s, size64 n, char* c);
size64 mstr_cpy(MStr* s, char* cs, size64 n);

/* Initialization */
void rl_init_str(void);

#endif
