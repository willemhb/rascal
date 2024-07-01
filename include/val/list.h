#ifndef rl_val_list_h
#define rl_val_list_h

#include "val/object.h"

/* Types, APIs, and globals supporting Rascal lists and pairs. */

/* C types. */
struct Pair {
  HEADER;

  Val car;
  Val cdr;
};

struct List {
  HEADER;

  Val    head;
  List*  tail;
  size_t cnt;
};

struct MPair {
  HEADER;

  Val car;
  Val cdr;
};

struct MList {
  HEADER;

  // data fields
  Val    head;
  MList* tail; // no count is stored, since it's impossible to ensure its accuracy with a mutable tail
};

/* Globals */
// types
extern List EmptyList;

extern Type PairType, ListType, MPairType, MListType;

/* External APIs */
/* List API */
#define is_list(x) has_type(x, &ListType)
#define as_list(x) ((List*)as_obj(x))

List*   mk_list(size_t n, Val* a);
List*   c_list(Val h, List* tl);
Val     list_ref(List* x, size_t n);
size_t  push_list(List* x, MVec* v);

/* Pair API */
#define is_pair(x) has_type(x, &PairType)
#define as_pair(x) ((Pair*)as_obj(x))

Pair* mk_pair(Val a, Val d);

/* MList API */
#define is_mlist(x) has_type(x, &MListType)
#define as_mlist(x) ((MList*)as_obj(x))

/* MPair API */
#define is_mpair(x) has_type(x, &MPairType)
#define as_mpair(x) ((MPair*)as_obj(x))

MPair* mk_mpair(Val a, Val d);

#endif
