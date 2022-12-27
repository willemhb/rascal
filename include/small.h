#ifndef rl_small_h
#define rl_small_h

#include "num.h"

/* globals */
extern struct type_t SmallType;

/* API */
bool long_fits_small(long l);
bool ulong_fits_small(ulong u);
bool uint_fits_small(uint u);
bool real_fits_small(double d);
bool float_fits_small(float f);

#define fits_small(x)                           \
  _Generic((x),                                 \
           long:long_fits_small,                \
           ulong:ulong_fits_small,              \
           uint:uint_fits_small,                \
           double:real_fits_small,              \
           float:float_fits_small)(x)

/* convenience */
#define is_small(x) has_type(x, &SmallType)
#define as_small(x) (((rl_data_t)(x)).as_small)

#endif
