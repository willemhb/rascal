#include "lang/compare.h"

#include "val/type.h"

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
    Type* xt = type_of(x), * yt = type_of(y);

    if ( xt == yt && xt->egal_fn )
      r = xt->egal_fn(x, y);
  }

  return r;
} 

int rl_order(Val x, Val y) {
  int r = x == y;

  if ( r != 0 ) {
    Type* xt = type_of(x), * yt = type_of(y);

    if ( xt != yt )
      r = cmp(xt->idno, yt->idno);

    else if ( xt->ord_fn )
      r = xt->ord_fn(x, y);
  }

  return r;
}

hash_t rl_hash(Val x, bool f) {
  hash_t h;

  if ( f )
    h = hash_word(x);

  else {
    Type* xt = type_of(x);

    if ( xt->hash_fn )
      h = xt->hash_fn(x);

    else
      h = hash_word(x & xt->val_mask);

    h = mix_hashes(h, xt->hash);
  }

  return h;
}
