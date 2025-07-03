#ifndef rl_data_types_port_h
#define rl_data_types_port_h

/* simple wrapper around a C file object. */
// headers --------------------------------------------------------------------
#include <stdio.h>

#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------
struct Port {
  HEAD;

  FILE* ios;
};

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Port* mk_port(FILE* ios);

// initialization -------------------------------------------------------------
void toplevel_init_data_type_port(void);

#endif
