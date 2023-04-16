#include "lang/hash.h"

#include "data/object.h"

#include "util/hashing.h"


// magic numbers
#define HASH_BOUND 16384

// APIs
uhash val_hash(value_t vx) {
  if (is_object(vx))
    return obj_hash(as_object(vx));

  return hash_uword(vx);
}

uhash obj_hash(void* ox) {
  assert(ox);

  uhash out;

  if (has_flag(ox, HASHED)) {
    out = head(ox)->hash;
  } else {
    
  }
}

uhash bounded_hash(void* ox, uhash* accum, int* bound) {
  assert(ox);

  if (has_flag(ox, HASHED)) {
    if (*accum)
      *accum = mix_2_hashes(head(ox)->hash, );
  }
}

// globals
extern uhash hash_symbol(void* ox);
extern uhash hash_binary(void* ox);

uhash TypeHashes[NTYPES];

uhash (*Hash[NTYPES])(void* ox) = {
  [SYMBOL] = hash_symbol,
  [BINARY] = hash_binary
};

extern uhash hash_list(void* ox, uhash* accum, int* bound);

// initialization
void hash_init(void) {
  for (type_t t=SYMBOL; t <=TOP; t++)
    TypeHashes[t] = hash_uword(t);
}
