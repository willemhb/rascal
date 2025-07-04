/* Currently a dummy value used to mark eg missing values in tables.

   May serve as the template for a bottom type in a more sophisticated type system. */
// headers --------------------------------------------------------------------
#include "data/none.h"

#include "lang/io.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void print_none(Port* ios, Expr x);

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------
void print_none(Port* ios, Expr x) {
  (void)x;
  pprintf(ios, "none");
}

// external -------------------------------------------------------------------

// initialization -------------------------------------------------------------
void toplevel_init_data_none(void) {
  Types[EXP_NONE] = (ExpTypeInfo) {
    .type     = EXP_NONE,
    .name     = "none",
    .obsize   = 0,
    .print_fn = print_none   
  };
}
