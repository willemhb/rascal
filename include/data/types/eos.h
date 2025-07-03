#ifndef rl_data_types_eos_h
#define rl_data_types_eos_h

/* Unique end of stream marker.

   Not clear that this should be its own type, I guess we'll see. */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------
#define EOS 0x7fff0000fffffffful

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// initialization -------------------------------------------------------------
void toplevel_init_data_type_eos(void);

#endif
