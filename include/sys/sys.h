#ifndef rl_sys_sys_h
#define rl_sys_sys_h

/**
 *
 * Bundles includes from the `sys` directory and exports the
 * toplevel initialization function.
 * 
 **/

// headers --------------------------------------------------------------------
#include "sys/base.h"
#include "sys/opcode.h"
#include "sys/error.h"
#include "sys/memory.h"
#include "sys/vm.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// initialization -------------------------------------------------------------
void toplevel_init_sys(void);

#endif
