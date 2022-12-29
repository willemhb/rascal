#include <assert.h>

#include "num.h"

#include "func.h"
#include "native.h"
#include "real.h"
#include "small.h"
#include "sym.h"

#include "type.h"

/* globals */
bool isa_num(type_t self, val_t val);

struct type_t NumType = {
  .name="num",
  .isa=isa_num
};

/* API */
/* internal */
bool isa_num(type_t self, val_t val) {
  self = type_of(val);

  return self == &RealType || self == &SmallType;
}


/* native functions */
/* helpers */
real_t get_real(val_t x) {
  assert(is_num(x));

  if (is_small(x))
    return as_small(x);

  return as_real(x);
}

val_t negate(val_t x) {
  assert(is_num(x));

  if (is_small(x))
    return tag_val(-as_small(x), SMALL);

  return as_val(-as_real(x));
}

/* guards */
func_err_t num_guard(size_t nargs, val_t *args) {
  for (size_t i=0; i<nargs; i++) {
    if (!has_type(args[i], &NumType))
      return func_arg_type_err;
  }

  return func_no_err;
}

func_err_t div_guard(size_t nargs, val_t *args) {
  for (size_t i=0; i<nargs; i++) {
    if (!has_type(args[i], &NumType))
      return func_arg_type_err;

    if (i > 0 && get_real(args[i]) == 0)
      return func_arg_value_err;
  }

  return func_no_err;
}

#define ALWAYS_TRUE(x) true
#define NON_ZERO(x) (!!(x))

#define ARITHMETIC(_op, _nargs, _args, _init, _n, _guard)   \
  small_t accum_s = _init; size_t i;                        \
                                                            \
  for (i=_n; i<_nargs && _guard(accum_s); i++) {            \
    if (is_real(_args[i]))                                  \
      goto reals;                                           \
    accum_s = accum_s _op as_small(_args[i]);               \
  }                                                         \
  return tag_val(accum_s, SMALL);                           \
reals:                                                      \
 real_t accum_r = accum_s;                                  \
 for (;i<_nargs;i++)                                        \
   accum_r = accum_r _op get_real(_args[i]);                \
 return as_val(accum_r);                                    \
 
val_t native_add(size_t nargs, val_t *args) {
  ARITHMETIC(+, nargs, args, 0, 0, ALWAYS_TRUE);
}

val_t native_sub(size_t nargs, val_t *args) {
  if (nargs == 1)
    return negate(args[0]);

  ARITHMETIC(-, nargs, args, args[0], 1, ALWAYS_TRUE);
}

val_t native_mul(size_t nargs, val_t *args) {
  ARITHMETIC(*, nargs, args, args[0], 1, NON_ZERO);
}

val_t native_div(size_t nargs, val_t *args) {
  if (nargs == 1)
    return as_val(1.0 / get_real(args[0]));

  ARITHMETIC(/, nargs, args, args[0], 1, NON_ZERO);
}

/* initialization */
void num_init(void) {
  val_t add = native("+", 1, true, num_guard, NULL, native_add);
  val_t sub = native("-", 1, true, num_guard, NULL, native_sub);
  val_t mul = native("*", 1, true, num_guard, NULL, native_mul);
  val_t div = native("/", 1, true, div_guard, NULL, native_div);

  define("+", add);
  define("-", sub);
  define("*", mul);
  define("/", div);
}
