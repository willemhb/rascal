
/**
 *
 * Idk it's the scanner dawg.
 * 
 **/

// headers --------------------------------------------------------------------
#include "sys/scanner.h"

#include "lang/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void init_scanner(Scanner* scn, Port* ios) {
  // rewind file
  prewind(ios);

  scn->ios  = ios;
  scn->line = 1;
}

// initialization -------------------------------------------------------------
void toplevel_init_sys_scanner(void);
