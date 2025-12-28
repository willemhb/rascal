#ifndef rl_list_h
#define rl_list_h

#include "val/val.h"

struct List {
  HEAD;
  Expr head;
  List* tail;
  int count;
  int line;
};

// list API
List*  empty_list(RlState* rls);
List*  empty_list_s(RlState* rls);
List*  mk_list(RlState* rls, int n);
List*  mk_list_s(RlState* rls, int n);
List*  cons(RlState* rls, Expr hd, List* tl);
List*  cons_s(RlState* rls, Expr hd, List* tl);
List*  cons_n(RlState* rls, int n);
List*  cons_n_s(RlState* rls, int n);
Expr   list_ref(List* xs, int n);
int    push_list(RlState* rls, List* xs);

// convenience macros and accessors
static inline Expr list_hd(List* xs) {
  return xs ? xs->head : NONE;
}

static inline List* list_tl(List* xs) {
  return xs ? xs->tail : NULL;
}

static inline int list_count(List* xs) {
  return xs ? xs->count : 0;
}

#define as_list(x)        ((List*)as_obj(x))
#define as_list_s(rls, x) ((List*)as_obj_s(rls, &ListType, x))

#define list_fst(l) ((l)->head)
#define list_snd(l) ((l)->tail->head)
#define list_thd(l) ((l)->tail->tail->head)
#define list_fth(l) ((l)->tail->tail->tail->head)

#endif
