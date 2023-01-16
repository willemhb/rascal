#ifndef rl_tpl_impl_funcall_h
#define rl_tpl_impl_funcall_h

#include "small.h"

#include "util/number.h"

#define TYPE_GUARD(_type, _args, _n)            \
  if (!is_##_type(_args[_n]))                   \
    return func_arg_type_err

#define ISA_GUARD(_type, _args, _n)		\
  if (!has_type(_args[0], _type))		\
    return func_arg_type_err

#define INDEX_GUARD(_len, _args, _n)                  \
  TYPE_GUARD(small, _args, _n);                       \
  small_t i = as_small(_args[_n]);                    \
  if (i < 0)                                          \
    i += (_len);                                      \
  if (i < 0 || (size_t)i >= (_len))                   \
    return func_arg_value_err

#define FALLBACK_NATIVE_CONSTRUCTOR(_type)				\
  func_err_t _type##_constructor_guard(size_t nargs, val_t *args) {	\
    (void)nargs;							\
    TYPE_GUARD(_type, args, 0);						\
    return func_no_err;							\
  }									\
  val_t native_##_type(size_t nargs, val_t *args) {			\
    (void)nargs;							\
    return args[0];							\
  }

#define DEF_FALLBACK_NATIVE_CONSTRUCTOR(_type, _Type)			\
  def_native(#_type, 1, false, _type##_constructor_guard, &_Type, native_##_type)

#endif
