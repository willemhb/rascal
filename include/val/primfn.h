#ifndef rl_val_primfn_h
#define rl_val_primfn_h

#include "val/func.h"

/* C types */
struct PrimFn {
  FN_HEADER;

  CPrimFn fn;
};

/* Globals */

/* API */
#define is_pfn(x) has_type(x, T_PRIMFN)
#define as_pfn(x) ((PrimFn*)as_obj(x))



/* Initialization */

#endif
