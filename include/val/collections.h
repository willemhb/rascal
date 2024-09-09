#ifndef rl_val_collections_h
#define rl_val_collections_h

#include "val/object.h"

#include "util/text.h"

/* Mutable collection types (mostly internal but exposed to users). */

/* C types */
struct Buffer {
  HEADER;

  CType ct; // C type of the stored data.
  bool  mb; // whether multibyte sequences exist.

  size_t cnt, cap;

  void* arr;
};

struct AList {
  HEADER;

  bool raw;

  size_t cnt, cap;

  union {
    void** objs;
    Val*   vals;
  };
};

struct TNode {
  Val    key, val; // key/value pair
  TNode* next;     // next node (for collisions)
};

struct Table {
  HEADER;

  size_t cnt, cap;

  TNode* tbl, *nxt, *end;
};

/* APIs */

#endif
