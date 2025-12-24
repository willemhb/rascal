#ifndef rl_list_h
#define rl_list_h

#include "val/val.h"

struct List {
  HEAD;
  Expr   head;
  List*  tail;
  size_t count;
};

// list API
List*  empty_list(RlState* rls);
List*  empty_list_s(RlState* rls);
List*  mk_list(RlState* rls, size_t n, Expr* xs);
List*  mk_list_s(RlState* rls, size_t n, Expr* xs);
List*  cons(RlState* rls, Expr hd, List* tl);
List*  cons_s(RlState* rls, Expr hd, List* tl);
List*  cons_n(RlState* rls, int n);
List*  cons_n_s(RlState* rls, int n);
Expr   list_ref(List* xs, int n);
int    push_list(RlState* rls, List* xs);

// convenience macros
#define as_list(x)           ((List*)as_obj(x))
#define as_list_s(rls, f, x) ((List*)as_obj_s(rls, f, &ListType, x))

#define list_fst(l) ((l)->head)
#define list_snd(l) ((l)->tail->head)
#define list_thd(l) ((l)->tail->tail->head)
#define list_fth(l) ((l)->tail->tail->tail->head)

#endif
