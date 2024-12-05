#include "lang/compare.h"

#include "val/object.h"

#include "vm/type.h"

#include "util/number.h"
#include "util/hash.h"

/* External APIs */
bool rl_same(Val x, Val y) {
  // simple pointer comparison
  return x == y;
}

bool rl_egal(Val x, Val y) {
  // deep comparison
  bool r = x == y;

  if ( r == false ) {
    VTable* xt = vtbl(x), * yt = vtbl(y);

    if ( xt->code == yt->code && egalfn(xt) )
      r = egalfn(xt)(x, y);
  }

  return r;
}

int rl_order(Val x, Val y) {
  int r = x == y;

  if ( r != 0 ) {
    VTable* xt = vtbl(x), * yt = vtbl(y);

    if ( xt->code != yt->code )
      r = cmp(xt->code, yt->code);

    else if ( orderfn(xt) )
      r = orderfn(xt)(x, y);
  }

  return r;
}

hash_t rl_hash(Val x) {
  hash_t h;

  if ( is_obj(x) ) {
    Obj* o = as_obj(x);

    if ( o->nohash || !o->sealed )
      h = hash_pointer(o);

    else {
      if ( o->hash == 0 ) {
        VTable* vt = vtbl(o);
        HashFn hf  = hashfn(o->tag);

        if ( hf == NULL )
          o->hash = hash_pointer(o);

        else
          o->hash = mix_hashes(thash(vt), hashfn(vt)(x));
      }
      
      h = o->hash;
    }
  } else {
    HashFn hf = hashfn(x);

    if ( hf )
      h = hf(x);

    else
      h = hash_word(x);
  }

  return h;
}
