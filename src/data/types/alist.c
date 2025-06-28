
/* Implementation for dynamic list utility type. */
// headers --------------------------------------------------------------------
#include "data/types/alist.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void trace_alist(void* ptr);
void free_alist(void* ptr);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void trace_alist(void* ptr) {
  Alist* a = ptr;

  trace_exprs(&a->exprs);
}

void free_alist(void* ptr) {
  Alist* a = ptr;
  
  free_exprs(&a->exprs);
}

// external -------------------------------------------------------------------
Alist* mk_alist(void) {
  Alist* out = mk_obj(EXP_ALIST, 0); init_exprs(&out->exprs);

  return out;
}

int alist_push(Alist* a, Expr x) {
  exprs_push(&a->exprs, x);

  return a->exprs.count;
}

Expr alist_pop(Alist* a) {
  return exprs_pop(&a->exprs);
}

Expr alist_get(Alist* a, int n) {
  assert(n >= 0 && n < a->exprs.count);

  return a->exprs.vals[n];
}

// initialization -------------------------------------------------------------
void toplevel_init_data_type_alist(void) {
  Types[EXP_ALIST] = (ExpTypeInfo){
    .type     = EXP_ALIST,
    .name     = "alist",
    .obsize   = sizeof(Alist),
    .trace_fn = trace_alist,
    .free_fn  = free_alist
  };
}

