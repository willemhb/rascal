#include <string.h>

#include "val/text.h"
#include "val/table.h"

#include "vm/heap.h"

/* Globals */

/* Internal APIs */
void intern_in_scache(void* t, void* e, void* k,  void* s, hash_t h) {
  (void)t;

  size_t n      = *(size_t*)s;
  SCEntry* ce   = e;
  ce->val       = new_str(k, n);
  ce->key       = ce->val->chars;
  ce->val->hash = h;
}

/* External APIs */
/* String API */
Str* get_str(char* cs, size_t n) {
  SCEntry* e;
  
  scache_intern(&StrCache, cs, &e, intern_in_scache, &n);

  return e->val;
}

Str* new_str(char* cs, size_t n) {
  Str* out   = new_obj(&StrType);
  out->enc   = ASCII;
  out->hasmb = false;
  out->chars = duplicate(cs, n+1, false);

  return out;
}

Glyph str_ref(Str* s, size_t n) {
  assert(n < s->count); // should already be checked

  return s->chars[n];
}

Str* str_set(Str* s, size_t n, Glyph g) {
  assert(n < s->count); // should already be checked

  char buf[s->count+1] = {};

  strcpy(buf, s->chars);

  buf[n] = g;

  return get_str(buf, s->count);
}

