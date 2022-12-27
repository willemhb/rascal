#ifndef rl_comp_h
#define rl_comp_h

#include "rascal.h"

/* C types */
typedef enum stx_err_t stx_err_t;

enum stx_err_t {
  no_stx_err,
  malformed_stx_err,
  illegal_stx_err,
};

/* API */
module_t comp(val_t x);

/* initialization */

#endif
