#ifndef rl_val_bool_h
#define rl_val_bool_h

#include "val/value.h"

/* C types */

/* Globals */
#define TRUE       0xffff000200000001ul // BOOL  |  1
#define FALSE      0xffff000200000000ul // BOOL  |  0

/* API */
#define is_bool(x) (wtag_bits(x) == BOOL)
#define as_bool(x) ((x) == TRUE)

bool is_truthy(Val x);

/* Initialization */

#endif
