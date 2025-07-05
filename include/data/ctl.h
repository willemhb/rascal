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

  Ctl* captured;
  Fun* fn;
  instr_t* pc;
  int fp, bp;
  Exprs stack; // copy of stack slice
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// initialization -------------------------------------------------------------
void toplevel_init_data_ctl(void);

#endif
