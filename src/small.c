#include <assert.h>

#include "small.h"

#include "real.h"

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
small_t get_small(val_t x) {
  assert(is_num(x));

  if (is_small(x))
    return as_small(x);

  return as_real(x);
}

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

/* natives */
#include "sym.h"
#include "native.h"

#include "tpl/impl/funcall.h"

func_err_t small_constructor_guard(size_t nargs, val_t *args) {
  (void)nargs;

  ISA_GUARD(&NumType, args, 0);

  return func_no_err;
}

val_t native_small(size_t nargs, val_t *args) {
  (void)nargs;

  small_t out = get_small(args[0]);

  return tag_val(out, SMALL);
}

/* initialization */
void small_init(void) {
  def_native("small", 1, false, small_constructor_guard, &SmallType, native_small);
}
