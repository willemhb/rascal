#include "lang.h"
#include "value.h"
#include "util.h"

/* top-level APIs. */
bool rl_same(Value x, Value y) {
  // simple pointer comparison
  return x == y;
}

bool rl_egal(Value x, Value y) {
  // pointer comparison for scalar or immutable values, otherwise deep comparison
  bool output;

  if (x == y) {
    output = true;
  } else {
    Type* xt = type_of(x), * yt = type_of(y);

    if ( xt != yt )
      output = false;

    else if ( xt->egal_fn == NULL )
      output = false;

    else
      output = xt->egal_fn(x, y);
  }

  return output;
}

int rl_order(Value x, Value y) {
  int output;

  if (x == y) {
    output = 0;
  } else {
    Type* xt = type_of(x), * yt = type_of(y);

    if ( xt != yt )
      output = 0 - (xt->idno < yt->idno ) + (xt->idno > yt->idno);

    else if ( xt->order_fn == NULL )
      output = 0 - (untag(x) < untag(y)) + (untag(x) > untag(y));

    else
      output = xt->order_fn(x, y);
  }

  return output;
}

hash_t rl_hash(Value x, bool fast) {
  hash_t output;

  if ( fast )
    output = hash_word(x);

  else {
    Type* xt = type_of(x);

    if ( xt->hash_fn == NULL )
      output = hash_word(x);

    else
      output = xt->hash_fn(x);
  }

  return output;
}
