#ifndef rl_val_list_h
#define rl_val_list_h

#include "val/object.h"

/* Types, APIs, and globals supporting Rascal lists and pairs. */

/* C types. */
struct Pair {
  HEADER;
  uint8 proper;
  
  Val car;

  size64 arity;
  
  union {
    Val   cdr;
    Pair* tail;
  };
};

/* External APIs */
/* List API */
#define is_cons(x) has_type(x, T_CONS)
#define as_cons(x) ((Pair*)as_obj(x))

Pair* mk_cons(Val car, Val cdr);
Pair* mk_list2(Val hd, Pair* tl);
Pair* mk_listn(size32 n, Val* d);
Val   list_ref(Pair* xs, size32 n);

#define mk_list(x, y)                           \
  generic((y),                                  \
          Val*:mk_listn,                        \
          Pair*:mk_list2)(x, y)

#endif
