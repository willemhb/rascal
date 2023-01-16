#ifndef rl_real_h
#define rl_real_h

#include "num.h"

/* globals */
extern struct type_t RealType;

/* API */
/* external */
real_t get_real(val_t x);

/* convenience */
#define is_real(x) has_type(x, &RealType)
#define as_real(x) (((rl_data_t)(x)).as_real)


#endif
