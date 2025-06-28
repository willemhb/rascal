
/*  Special type for indirecting captured references, used to implement closures. */
// headers --------------------------------------------------------------------

#include "data/types/upv.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void trace_upval(void* ptr);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void trace_upval(void* ptr) {
  UpVal* upv = ptr;

  if ( upv->closed )
    mark_exp(upv->val);
}

// external -------------------------------------------------------------------
UpVal* mk_upval(UpVal* next, Expr* loc) {
  // only open upvalues can be created
  UpVal* upv  = mk_obj(EXP_UPV, 0);
  upv->next   = next;
  upv->closed = false;
  upv->loc    = loc;

  return upv;
}

Expr* deref(UpVal* upv) {
  return upv->closed ? &upv->val : upv->loc;
}

// initialization -------------------------------------------------------------
void toplevel_init_data_type_upv(void) {
  Types[EXP_UPV] = (ExpTypeInfo) {
    .type     = EXP_UPV,
    .name     = "upval",
    .obsize   = sizeof(UpVal),
    .trace_fn = trace_upval  
  };
}
