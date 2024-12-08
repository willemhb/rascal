#ifndef rl_val_num_h
#define rl_val_num_h

#include "val/value.h"

/* C types */

/* Globals */

/* API */
#define is_num(x)   has_type(x, T_NUM)
#define as_num(x)   num_val(x)
#define is_small(x) (wtag_bits(x) == SMALL)
#define as_small(x) wdata_bits(x)

Num num_val(Val x);

/* Initialization */

#endif
