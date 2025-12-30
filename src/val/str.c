#include <string.h>

#include "val/str.h"
#include "val/port.h"
#include "vm.h"
#include "util/util.h"

// forward declarations
void print_str(Port* ios, Expr x);
hash_t hash_str(Expr x);
bool egal_strs(Expr x, Expr y);
void free_str(RlState* rls, void* ptr);

// Type object
Type StrType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_STR,
  .obsize   = sizeof(Str),
  .print_fn = print_str,
  .hash_fn  = hash_str,
  .egal_fn  = egal_strs,
  .free_fn  = free_str
};

// string table
Strings StringTable = {
  .kvs   = NULL,
  .count = 0,
  .maxc  = 0
};

Str* new_str(RlState* rls, char* cs, hash_t h, bool interned) {
  Str* s = mk_obj(rls, &StrType, 0);
  s->val = duplicates(rls, cs);
  s->count = strlen(cs);
  s->hash = h;
  s->flags = interned;

  return s;
}

void string_intern(RlState* rls, Strings* t, StringsKV* kv, char* k, hash_t h) {
  (void)t;

  Str* s  = new_str(rls, k, h, true);
  kv->val = s;
  kv->key = s->val;
}

Str* mk_str(RlState* rls, char* cs) {
  size_t n = strlen(cs);
  Str* s;

  if ( n <= MAX_INTERN )
    s = strings_intern(rls, &rls->vm->strings, cs, string_intern);

  else
    s = new_str(rls, cs, hash_string_48(cs), false);

  return s;
}

Str* mk_str_s(RlState* rls, char* cs) {
  Str* out = mk_str(rls, cs);
  stack_push(rls, tag_obj(out));
  return out;
}

void print_str(Port* ios, Expr x) {
  Str* s = as_str(x);

  pprintf(ios, "\"%s\"", s->val);
}

hash_t hash_str(Expr x) {
  Str* s = as_str(x);

  if ( s->hash == 0 )
    s->hash = hash_string_48(s->val);

  return mix_hashes_48(StrType.hashcode, s->hash);
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

void free_str(RlState* rls, void* ptr) {
  Str* s = ptr;

  if ( is_interned(s) ) // make sure to remove from Strings table before freeing
    strings_del(rls, &StringTable, s->val, NULL);

  release(rls, s->val, 0);
}
