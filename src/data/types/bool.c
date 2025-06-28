/* DESCRIPTION */
// headers --------------------------------------------------------------------
#include "data/types/bool.h"

#include "lang/io.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void print_bool(Port* ios, Expr x);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void print_bool(Port* ios, Expr x) {
  pprintf(ios, x == TRUE ? "true" : "false");
}

// external -------------------------------------------------------------------
Bool as_bool(Expr x) {
  return x == TRUE;
}

Expr tag_bool(Bool b) {
  return b ? TRUE : FALSE;
}

// initialization -------------------------------------------------------------
void toplevel_init_data_type_bool(void) {
  Types[EXP_BOOL] = (ExpTypeInfo) {
    .type     = EXP_BOOL,
    .name     = "bool",
    .obsize   = 0,
    .print_fn = print_bool
  };
}
