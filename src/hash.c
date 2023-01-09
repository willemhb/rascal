#include "hash.h"

#include "type.h"

#include "util/hashing.h"

/* API */
ulong hash_dispatch(val_t x, void *state) {
  type_t tx = type_of(x);

  if (tx->hash)
    return tx->hash(x, state);

  return hash_uint(x);
}

ulong hash(val_t x) {
  type_t tx = type_of(x);

  if (tx->hash)
    return tx->hash(x, NULL);

  return hash_uint(x);
}
