#ifndef rl_data_nul_h
#define rl_data_nul_h

/* Unit type. Pretty self explanatory. */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------
#define NUL  0x7ffe000000000000ul // NUL_T | 0
#define NONE 0x7ffe000000000001ul // NUL_T | 1 (not a valid value, used as a sentinel)

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// initialization -------------------------------------------------------------
void toplevel_init_data_nul(void);

#endif
