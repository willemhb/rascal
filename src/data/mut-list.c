
/* Implementation for dynamic list utility type. */
// headers --------------------------------------------------------------------
#include "data/mut-list.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void trace_mut_list(void* ptr);
void free_mut_list(void* ptr);

// globals --------------------------------------------------------------------
ExpAPI MutListExpAPI = {};

ObjAPI MutListObjAPI = {
  .obsize   = sizeof(MutList),
  .trace_fn = trace_mut_list,
  .free_fn  = free_mut_list,
};

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void trace_mut_list(void* ptr) {
  MutList* a = ptr;

  trace_exprs(&a->exprs);
}

void free_mut_list(void* ptr) {
  MutList* a = ptr;
  
  free_exprs(&a->exprs);
}

// external -------------------------------------------------------------------
MutList* mk_mut_list(void) {
  MutList* out = mk_obj(EXP_MUT_LIST, 0, 0); init_exprs(&out->exprs);

  return out;
}

int mut_list_push(MutList* a, Expr x) {
  exprs_push(&a->exprs, x);

  return a->exprs.count;
}

Expr mut_list_pop(MutList* a) {
  return exprs_pop(&a->exprs);
}

Expr mut_list_get(MutList* a, int n) {
  assert(n >= 0 && n < a->exprs.count);

  return a->exprs.vals[n];
}

// initialization -------------------------------------------------------------
void toplevel_init_data_mut_list(void) {
  Types[EXP_MUT_LIST] = (TypeInfo) {
    .type    = EXP_MUT_LIST,
    .c_name  = "mut_list",
    .exp_api = &MutListExpAPI,
    .obj_api = &MutListObjAPI
  };
}

