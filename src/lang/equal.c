#include "util/hashing.h"

#include "lang/equal.h"

#include "val/type.h"


/* External API */
bool same(Value x, Value y) {
  /* pointer comparison. */
  return x == y;
}

bool equal(Value x, Value y) {
  /* Value comparison.

     Only immutable objects whose types define an egalfn
     are compared deeply.

     The rascal function `=?` corresponding to `equal` is
     is declared a singleton and automatically defined
     for . */

  bool out;
  
  if (x == y)
    out = true;

  else if (!is_obj(x) || !is_obj(y))
    out = false;

  else if (is_editp(x) || is_editp(y))
    out = false;

  else {
    Type* xt = type_of(x), * yt = type_of(y);

    if (xt != yt)
      out = false;

    else if (xt->vtable->egalfn)
      out = xt->vtable->egalfn(x, y);

    else
      out = false;
  }

  return out;
}

hash_t hash(Value x) {
  hash_t out;
  
  if (!is_obj(x))
    out = hash_word(x);

  else {
    Obj* o = as_obj(x);
    Type* t = o->type;

    if (!is_hashed(o)) {
      /* object hashes computed at most once. */
      hash_t ohash;

      if (t->vtable->hashfn)
        ohash = t->vtable->hashfn(x);

      else
        ohash = hash_ptr(o);

      o->hash   = ohash;
      o->memfl |= HASHED;
    }

    /* munge object hash with type to distinguish eg `()` and `[]`. */
    out = mix_hashes(t->obj.hash, o->hash);
  }

  return out;
}
