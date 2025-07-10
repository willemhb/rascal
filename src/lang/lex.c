/* DESCRIPTION */
// headers --------------------------------------------------------------------
#include <string.h>

#include "lang/lex.h"

#include "sys/error.h"
#include "sys/memory.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------
char ScanBuffer[BUFFER_SIZE];

Scanner Lexer = {
  .ios   =NULL,
  .line  =0,
  .offset=0,
  .buffer=ScanBuffer
};

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------


// external -------------------------------------------------------------------
void init_scanner(Scanner* scn, Port* ios) {
  prewind(ios);

  // initialize fields
  scn->ios  = ios;
  scn->line = 1;

  // scrub old data
  memset(scn->buffer, 0, BUFFER_SIZE*sizeof(char));
}

// initialization -------------------------------------------------------------
