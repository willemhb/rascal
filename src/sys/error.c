/**
 *
 * Rascal builtin error handling support.
 * 
 **/

// headers --------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "sys/error.h"

#include "data/types/sym.h"

#include "lang/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------
void panic(Status etype) {
  if ( etype == SYSTEM_ERROR )
    exit(1);

  longjmp(SaveState, etype);
}

void recover(funcptr_t cleanup) {
  if ( cleanup )
    cleanup();
}

void rascal_error(Status etype, char* fmt, ...) {
  if ( etype > USER_ERROR ) { // user error messages handled in rascal code
    va_list va;
    va_start(va, fmt);
    pprintf(&Errs, "%s error: ", ErrorNames[etype]);
    pvprintf(&Errs, fmt, va);
    pprintf(&Errs, ".\n");
    va_end(va);
  }

  panic(etype);
}

// initialization -------------------------------------------------------------
static void toplevel_init_error_types(void) {
  char buffer[256] = { ':' };

  for ( int i=0; i < NUM_ERRORS; i++ ) {
    strcpy(buffer+1, ErrorNames[i]);

    ErrorTypes[i] = mk_sym(buffer);
  }
}

void toplevel_init_sys_error(void) {
  toplevel_init_error_types();
}
