#include <assert.h>

#include "real.h"

#include "small.h"

#include "type.h"

/* globals */
void prin_real(val_t val);

struct type_t RealType = {
  .name="real",
  .prin=prin_real
};

/* API */
/* external */
real_t get_real(val_t x) {
  assert(is_num(x));

  if (is_small(x))
    return as_small(x);

  return as_real(x);
}

/* internal */
void prin_real(val_t val) {
  printf("%.2g", as_real(val));
}

/* natives */
#include "native.h"
#include "sym.h"

#include "tpl/impl/funcall.h"

func_err_t real_constructor_guard(size_t nargs, val_t *args) {
  (void)nargs;

  ISA_GUARD(&NumType, args, 0);

  return func_no_err;
}

val_t native_real(size_t nargs, val_t *args) {
  (void)nargs;

  real_t out = get_real(args[0]);

  return as_val(out);
}

/* initialization */
void real_init(void) {
  def_native("real", 1, false, real_constructor_guard, &RealType, native_real);
}
