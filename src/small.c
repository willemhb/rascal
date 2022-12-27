#include "small.h"

#include "type.h"

#include "util/number.h"

/* declarations */
void prin_small(val_t x);

/* globals */
struct type_t SmallType = {
  .name="small",
  .prin=prin_small,
};

/* API */
/* external */
bool long_fits_small(long l) {
  return l <= INT32_MAX && l >= INT32_MIN;
}

bool ulong_fits_small(ulong u) {
  return u <= INT32_MAX;
}

bool uint_fits_small(uint u) {
  return u <= INT32_MAX;
}

bool real_fits_small(double d) {
  return isint(d) && d <= INT32_MAX && d >= INT32_MIN;
}

bool float_fits_small(float f) {
  return isint(f) && f <= INT32_MAX && f >= INT32_MIN;
}

/* internal */
void prin_small(val_t x) {
  printf("%d", as_small(x));
}
