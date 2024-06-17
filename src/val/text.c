#include <string.h>

#include "val/text.h"
#include "val/table.h"

#include "vm/heap.h"

/* Forward declarations */
Str* new_str(char* cs, size_t n);

/* Globals */
// types
extern Type PortType, GlyphType, StrType, BinType, MStrType, MBinType, RTType;

// standard ports
extern Port StdIn, StdOut, StdErr;

// string cache
extern SCache StrCache;

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
Str* mk_str(char* cs, size_t n) {
  assert(cs != NULL);

  if ( n == 0 && *cs != '\0' )
    n = strlen(cs);

  SCEntry* e = scache_intern(&StrCache, cs, intern_in_scache, &n);

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

  return mk_str(buf, s->count);
}

/* Bin API */
Bin*   new_bin(size_t n, void* d, CType ct);
byte_t bin_ref(Bin* b, size_t n);
Bin*   bin_set(Bin* b, size_t n, byte_t u);

/* RT API */
RT* mk_rt(RT* p) {
  RT* o = new_obj(&RTType);

  init_rt(o, p);

  return o;
}

void init_rt(RT* rt, RT* p) {
  rt->parent = p;

  if ( p != NULL ) {
    // copy readers from parent read table
    rt->eof_fn = p->eof_fn;
    memcpy(rt->dispatch, p->dispatch, sizeof(rt->dispatch));
    memcpy(rt->intrasym, p->intrasym, sizeof(rt->intrasym));
  } else {
    // initialize everything to NULL (rt_get returns fail reader if result is NULL)
    rt->eof_fn = NULL;

    memset(rt->dispatch, 0, sizeof(rt->dispatch));
    memset(rt->intrasym, 0, sizeof(rt->intrasym));
  }
}

void rt_set_g(RT* rt, int d, rl_read_fn_t r, bool is) {
  assert(d == EOF || d < RT_SIZE); // require ASCII atm

  if ( d == EOF )
    rt->eof_fn = r;

  else if ( is )
    rt->intrasym[d] = r;

  else
    rt->dispatch[d] = r;
}

void rt_set_s(RT* rt, char* ds, rl_read_fn_t r, bool is) {
  assert(ds != NULL);

  while ( *ds ) {
    rt_set(rt, *ds, r, is);
    ds++;
  }
}

extern void rl_read_fail(RState* s, int d);

rl_read_fn_t rt_get(RT* rt, int d, bool is) {
  assert(d == EOF || d < RT_SIZE); // require ASCII atm

  rl_read_fn_t f;

  if ( d == EOF )
    f = rt->eof_fn;

  else if ( is )
    f = rt->intrasym[d] ? : rt->dispatch[d];

  else
    f = rt->dispatch[d];

  if ( f == NULL )
    f = rl_read_fail;

  return f;
}
