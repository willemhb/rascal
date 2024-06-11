#ifndef rl_list_h
#define rl_list_h

#include "val/object.h"

/* Types, APIs, and globals supporting Rascal lists and pairs. */

/* C types. */
struct Pair {
  HEADER;

  Value car;
  Value cdr;
};

struct List {
  HEADER;

  Value  head;
  List*  tail;
  size_t count;
};

struct MutPair {
  HEADER;

  Value car;
  Value cdr;
};

struct MutList {
  HEADER;

  // data fields
  Value    head;
  MutList* tail; // no count is stored, since it's impossible to ensure its accuracy with a mutable tail
};

/* Globals */
// types
extern Type PairType, ListType, MutPairType, MutListType;

#endif
