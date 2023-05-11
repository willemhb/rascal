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
  bool oob = false;
  uhash out = bounded_hash(ox, HASH_BOUND, &oob);
  return out;
}

uhash bounded_hash(void* ox, int bound, bool* oob) {
  assert(ox);

  uhash out;

  if (HASH(ox))
    out = HASH(ox)(ox, bound, oob);

  else
    out = hash_uword(object(ox));

  return out;
}

// globals
extern uhash hash_symbol(void* ox, int bound, bool* oob);
extern uhash hash_list(void* ox, int bound, bool* oob);
extern uhash hash_binary(void* ox, int bound, bool* oob);
extern uhash hash_table(void* ox, int bound, bool* oob);
extern uhash hash_vector(void* ox, int bound, bool* oob);
extern uhash hash_function(void* ox, int bound, bool* oob);
extern uhash hash_environment(void* ox, int bound, bool* oob);
extern uhash hash_chunk(void* ox, int bound, bool* oob);
extern uhash hash_closure(void* ox, int bound, bool* oob);
extern uhash hash_variable(void* ox, int bound, bool* oob);
extern uhash hash_upvalue(void* ox, int bound, bool* oob);

uhash TypeHashes[NTYPES];

uhash (*Hash[NTYPES])(void* ox, int bound, bool* oob) = {
  [SYMBOL]      = hash_symbol,
  [LIST]        = hash_list,
  [BINARY]      = hash_binary,
  [TABLE]       = hash_table,
  [VECTOR]      = hash_vector,
  [FUNCTION]    = hash_function,
  [ENVIRONMENT] = hash_environment,
  [CHUNK]       = hash_chunk,
  [CLOSURE]     = hash_closure,
  [VARIABLE]    = hash_variable,
  [UPVALUE]     = hash_upvalue
};

// initialization
void hash_init(void) {
  for (type_t t=SYMBOL; t <=TOP; t++)
    TypeHashes[t] = hash_uword(t);
}
