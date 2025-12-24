#include "val/alist.h"
#include "vm.h"

// forward declarations
void trace_alist(RlState* rls, void* ptr);

// Type object
Type AlistType = {
  .heap     = NULL,
  .type     = &TypeType,
  .bfields  = FL_GRAY,
  .tag      = EXP_ALIST,
  .obsize   = sizeof(Alist),
  .trace_fn = trace_alist,
  .free_fn  = free_alist
};

// alist API
Alist* mk_alist(RlState* rls) {
  Alist* out = mk_obj(rls, &AlistType, 0);
  init_exprs(rls, &out->exprs);

  return out;
}

Alist* mk_alist_s(RlState* rls) {
  Alist* out = mk_alist(rls);
  push(rls, tag_obj(out));

  return out;
}

int alist_push(RlState* rls, Alist* a, Expr x) {
  exprs_push(rls, &a->exprs, x);

  return a->exprs.count;
}

Expr alist_pop(RlState* rls, Alist* a) {
  return exprs_pop(rls, &a->exprs);
}

Expr alist_get(Alist* a, int n) {
  assert(n >= 0 && n < a->exprs.count);

  return a->exprs.vals[n];
}

void trace_alist(RlState* rls, void* ptr) {
  Alist* a = ptr;

  trace_exprs(rls, &a->exprs);
}

void free_alist(RlState* rls, void* ptr) {
  Alist* a = ptr;

  free_exprs(rls, &a->exprs);
}
