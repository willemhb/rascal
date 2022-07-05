#ifndef rascal_pair_h
#define rascal_pair_h

#include "core.h"

struct pair_t {
  object_t base;
  union {
    size_t len;
    hash_t hash;
  };
  value_t car, cdr;
};

#define aspair(x) ((pair_t*)ptr(x))
#define car(x)    (aspair(x)->car)
#define cdr(x)    (aspair(x)->cdr)
#define clen(x)   (aspair(x)->len)
#define phash(x)  (aspair(x)->hash)

#endif
