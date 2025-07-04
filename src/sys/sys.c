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
char* ErrorNames[NUM_ERRORS] = {
  [OKAY]          = "okay",
  [USER_ERROR]    = "user",
  [EVAL_ERROR]    = "eval",
  [RUNTIME_ERROR] = "runtime",
  [SYSTEM_ERROR]  = "sytem"
};

Sym* ErrorTypes[NUM_ERRORS] = {};

char Token[BUFFER_SIZE];
size_t TOff = 0;
Obj* Heap = NULL;
size_t HeapUsed = 0, HeapCap = INIT_HEAP;
GcFrame* GcFrames = NULL;

Expr Vals[N_VALS];
Expr Frames[N_FRAMES];

VM Vm = {
  .upvs   = NULL,
  .fn     = NULL,
  .pc     = NULL,
  .sp     = 0,
  .fp     = 0,
  .bp     = 0,
  .frames = Frames,
  .vals   = Vals
};

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------

// initialization -------------------------------------------------------------
void toplevel_init_sys(void) {
  toplevel_init_sys_vm();
  toplevel_init_sys_memory();
  toplevel_init_sys_error();
}
