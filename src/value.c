#include "hashing.h"
#include "value.h"
#include "symbol.h"
#include "rstring.h"

inline bool_t rtoCbool(value_t xv) {
  return !(nullp(xv) || falsep(xv));
}

type_t rtypeof(value_t xv) {
  if (nullp(xv))   return type_null;
  if (objectp(xv)) return obtype(xv);
  return imtype(xv);
}

size_t rsizeof(value_t xv) {
  if (immediatep(xv))
    return TypeSizes[imtype(xv)];

  return obsize(xv);
}

hash_t  rhash(value_t xv) {
  if (nullp(xv)) return pointerhash((void*)xv);
  if (immediatep(xv)) return int64hash(xv);      // includes the type as part of the hash
  if (symbolp(xv)) return symhash(xv);
  if (stringp(xv)) return strhash(schars(xv));
  return pointerhash(ptr(xv));
}

inline char_t *typename(value_t xv) {
  return TypeNames[rtypeof(xv)];
}
