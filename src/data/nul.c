/* Unit type. Pretty self explanatory. */
// headers --------------------------------------------------------------------
#include "data/nul.h"

#include "lang/io.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void print_nul(Port* ios, Expr x);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void print_nul(Port* ios, Expr x) {
  (void)x;
  pprintf(ios, "nul");
}

// external -------------------------------------------------------------------

// initialization -------------------------------------------------------------
void toplevel_init_data_nul(void) {
  Types[EXP_NUL] = (ExpTypeInfo) {
    .type     = EXP_NUL,
    .name     = "nul",
    .obsize   = 0,
    .print_fn = print_nul
  };
}
