#ifndef rl_data_bool_h
#define rl_data_bool_h

/* Boolean type. */
// headers --------------------------------------------------------------------
#include "common.h"

#include "data/base.h"

// macros ---------------------------------------------------------------------
#define TRUE   0xfffc000000000001ul // BOOL_T | 1
#define FALSE  0xfffc000000000000ul // BOOL_T | 0

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
Bool as_bool(Expr x);
Expr tag_bool(Bool b);

// initialization -------------------------------------------------------------
void toplevel_init_data_bool(void);

#endif
