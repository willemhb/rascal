#ifndef rl_val_list_h
#define rl_val_list_h

#include "val/object.h"

/* Types, APIs, and globals supporting Rascal lists and pairs. */

/* C types */
struct List {
  HEADER;

  Val    head;
  size64 cnt;
  List*  tail;
};

struct Pair {
  HEADER;

  Val car;
  Val cdr;
};

/* Globals */
extern List EmptyList;

/* External APIs */
// List API
#define is_list(x) has_type(x, T_LIST)
#define as_list(x) ((List*)as_obj(x))

#define mk_list(x, y)                           \
  generic((y),                                  \
          Val*:mk_listn,                        \
          List*:mk_list2)(x, y)

List* mk_list2(Val hd, List* tl);
List* mk_listn(size32 n, Val* d);
Val   list_ref(List* xs, size64 n);

// Pair API
#define is_pair(x) has_type(x, T_PAIR)
#define as_pair(x) ((Pair*)as_obj(x))

Pair* mk_pair(Val car, Val cdr);
Pair* mk_kv(Val k, hash64 h);

// initialization
void rl_toplevel_init_list(void);

#endif
