#ifndef rl_data_types_nul_h
#define rl_data_types_nul_h

/* Unit type. Pretty self explanatory. */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------
#define NUL 0x7ffe000000000000ul // NUL_T | 0

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// initialization -------------------------------------------------------------
void toplevel_init_data_type_nul(void);

#endif
