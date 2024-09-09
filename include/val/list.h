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

/* External APIs */
/* List API */
#define is_list(x) has_type(x, T_LIST)
#define as_list(x) ((List*)as_obj(x))

List*   mk_list(size_t n, Val* a);
List*   c_list(Val h, List* tl);
Val     list_ref(List* x, size_t n);
size_t  push_list(List* x, Alist* v);

/* Pair API */
#define is_pair(x) has_type(x, T_PAIR)
#define as_pair(x) ((Pair*)as_obj(x))

Pair* mk_pair(Val a, Val d);

#endif
