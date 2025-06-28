/* User string type.

   Small strings are interned, making them the basis for fast symbol comparison. */

// headers --------------------------------------------------------------------
#include <string.h>

#include "data/table.h"
#include "data/types/str.h"

#include "lang/io.h"

#include "util/hashing.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------
Strings StringTable = {
  .kvs       = NULL,
  .count     = 0,
  .max_count = 0
};

// function prototypes --------------------------------------------------------
void   print_str(Port* ios, Expr x);
hash_t hash_str(Expr x);
bool   egal_strs(Expr x, Expr y);
void   free_str(void* ptr);
Str*   new_str(char* cs, hash_t h, bool interned);
void   string_intern(Strings* t, StringsKV* kv, char* k, hash_t h);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void print_str(Port* ios, Expr x) {
  Str* s = as_str(x);

  pprintf(ios, "\"%s\"", s->val);
}

hash_t hash_str(Expr x) {
  Str* s = as_str(x);

  return s->hash;
}

bool egal_strs(Expr x, Expr y) {
  bool out;
  Str* sx = as_str(x), * sy = as_str(y);

  if ( sx->count != sy->count )
    out = false;

  else {
    if ( is_interned(sx) )
      out = sx == sy;

    else
      out = strcmp(sx->val, sy->val) == 0;
  }

  return out;
}

void free_str(void* ptr) {
  Str* s = ptr;

  if ( is_interned(s) ) // make sure to remove from Strings table before freeing
    strings_del(&StringTable, s->val, NULL);

  release(s->val, 0);
}

Str* new_str(char* cs, hash_t h, bool interned) {
  Str* s    = mk_obj(EXP_STR, 0);

  s->val    = duplicates(cs);
  s->count  = strlen(cs);
  s->hash   = h;
  s->flags  = interned;

  return s;
}

void string_intern(Strings* t, StringsKV* kv, char* k, hash_t h) {
  (void)t;

  Str* s  = new_str(k, h, true);
  kv->val = s;
  kv->key = s->val;
}

// external -------------------------------------------------------------------
Str* as_str_s(char* f, Expr x) {
  ExpType t = exp_type(x);
  require(t == EXP_STR, "%s wanted a str, got %s", f, Types[t].name);

  return as_str(x);
}

Str* mk_str(char* cs) {
  size_t n = strlen(cs);
  Str* s;

  if ( n <= MAX_INTERN )
    s = strings_intern(&StringTable, cs, string_intern);

  else
    s = new_str(cs, hash_string(cs), false);

  return s;
}

// initialization -------------------------------------------------------------
void toplevel_init_data_type_str(void) {
  Types[EXP_STR] = (ExpTypeInfo) {
    .type     = EXP_STR,
    .name     = "str",
    .obsize   = sizeof(Str),
    .print_fn = print_str,
    .hash_fn  = hash_str,
    .egal_fn  = egal_strs,
    .free_fn  = free_str
  };
}
