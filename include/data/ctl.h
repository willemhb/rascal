#ifndef rl_data_ctl_h
#define rl_data_ctl_h

/**
 *
 * Definition/API for type representing suspended continuation.
 *
 **/

// headers --------------------------------------------------------------------
#include "data/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
struct Ctl {
  HEAD;

  Fun* fn;
  instr_t* pc;
  
  Exprs stack;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// initialization -------------------------------------------------------------
void toplevel_init_data_ctl(void);

#endif
