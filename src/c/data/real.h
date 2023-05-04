#ifndef data_real_h
#define data_real_h

#include "data/value.h"

// APIs & utilities
#define is_real(x) rl_isa(x, REAL)
#define as_real(x) (((ieee64_t)(x)).dbl)

#endif
