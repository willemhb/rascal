#ifndef rascal_table_h
#define rascal_table_h

#include "rascal.h"

// table root structure
struct mapping_t {
  object_t base;
  uint_t len   : 24;
  uint_t red   :  1;
  uint_t flags :  7;
  uint_t arity : 24;
  uint_t _pad  :  8;

  hash_t hash;

  value_t key, val;
  object_t *parent, *left, *right;
};


#define asmap(x)     ((mapping_t*)ptr(x))
#define maplen(x)    (asmap(x)->len)
#define mapflags(x)  (asmap(x)->flags)
#define maparity(x)  (asmap(x)->arity)
#define maphash(x)   (asmap(x)->hash)
#define mapkey(x)    (asmap(x)->key)
#define mapval(x)    (asmap(x)->val)
#define mapleft(x)   (asmap(x)->left)
#define mapright(x)  (asmap(x)->right)

// api ------------------------------------------------------------------------
int_t   map_loc(object_t *ob, value_t key, object_t **spc );


#endif
