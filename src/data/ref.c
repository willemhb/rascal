
/* DESCRIPTION */
// headers --------------------------------------------------------------------

#include "data/ref.h"
#include "data/sym.h"

#include "lang/io.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void print_ref(Port* ios, Expr x);
void trace_ref(void* ptr);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void print_ref(Port* ios, Expr x) {
  Ref* r = as_ref(x);

  pprintf(ios, "#'%s", r->name->val->val);
}

void trace_ref(void* ptr) {
  Ref* r = ptr;

  mark_obj(r->name);
}

// external -------------------------------------------------------------------
Ref* mk_ref(Sym* n, int o) {
  Ref* ref  = mk_obj(EXP_REF, 0);
  ref->name = n;
  ref->ref_type = REF_UNDEF; // filled in by env_put, env_resolve, &c
  ref->offset = o;

  return ref;
}

// initialization -------------------------------------------------------------
void toplevel_init_data_ref(void) {
  Types[EXP_REF] = (ExpTypeInfo) {
    .type     = EXP_REF,
    .name     = "ref",
    .obsize   = sizeof(Ref),
    .print_fn = print_ref,
    .trace_fn = trace_ref
  };

  
}
