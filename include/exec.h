#ifndef rascal_exec_h
#define rascal_exec_h

#include "types.h"

// main driver for the VM goes in here, as well as related typedefs, &c
// opcodes
enum
  {
    // load/store instructions
    HALT    =0x00,
    CONSTANT=0x01,
    GLOBAL  =0x02,
  };

//

// forward declarations
void repl( void );

#endif
