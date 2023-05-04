#ifndef data_fixnum_h
#define data_fixnum_h

#include "data/value.h"

// APIs & utilities
#define is_fixnum(x)  rl_isa(x, FIXNUM)
#define as_fixnum(x)  rl_asa(x, WVMASK, fixnum_t)

#endif
