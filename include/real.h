#ifndef rl_real_h
#define rl_real_h

#include "num.h"

/* globals */
extern struct type_t RealType;

/* convenience */
#define is_real(x) has_type(x, &RealType)
#define as_real(x) (((rl_data_t)(x)).as_real)


#endif
