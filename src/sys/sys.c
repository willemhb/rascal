/**
 *
 * Bundles includes from the `sys` directory and exports the
 * toplevel initialization function.
 * 
 **/

// headers --------------------------------------------------------------------
#include <string.h>

#include "sys/sys.h"

#include "data/types/sym.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------

// initialization -------------------------------------------------------------
static void toplevel_init_error_types(void) {
  char buffer[256] = { ':' };

  for ( int i=0; i < NUM_ERRORS; i++ ) {
    strcpy(buffer+1, ErrorNames[i]);

    ErrorTypes[i] = mk_sym(buffer);
  }
}

void toplevel_init_sys(void) {
  toplevel_init_error_types();
}
