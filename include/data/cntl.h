#ifndef rl_data_cntl_h
#define rl_data_cntl_h

/**
 *
 * DESCRIPTION
 *
 **/

// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
struct Cntl {
  HEAD;

  Cntl* parent;

  int n_stack; /* size of stack slice */
  int n_cntl; /* size of control slice */
  CSRef* cntl_slice; /* preserved control state */
  StackRef* stack_slice; /* preserved stack state */
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// initialization -------------------------------------------------------------

#endif
