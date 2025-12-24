#ifndef rl_alist_h
#define rl_alist_h

#include "val/val.h"

// wrapper around a Stack object
struct Alist {
  HEAD;
  Exprs exprs;
};

// alist API
Alist* mk_alist(RlState* rls);
Alist* mk_alist_s(RlState* rls);
void   free_alist(RlState* rls, void* ptr);
int    alist_push(RlState* rls, Alist* a, Expr x);
Expr   alist_pop(RlState* rls, Alist* a);
Expr   alist_get(Alist* a, int n);

#endif
