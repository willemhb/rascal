#ifndef rl_data_types_none_h
#define rl_data_types_none_h

/* Currently a dummy value used to mark eg missing values in tables.

   May serve as the template for a bottom type in a more sophisticated type system. */
// headers --------------------------------------------------------------------
#include "common.h"
#include "data/expr.h"

// macros ---------------------------------------------------------------------
#define NONE 0x7ffd000000000000ul

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// initialization -------------------------------------------------------------
void toplevel_init_data_type_none(void);

#endif
