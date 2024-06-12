#include "val/type.h"

#include "util/hash.h"

/* Internal hashing utilities (mostly for circumventing unnecessary dispatch). */
hash_t get_type_hash(Type* type) {
  if ( type->hash == 0 )
    type->hash = hash_word(type->idno);

  return type->hash;
}
