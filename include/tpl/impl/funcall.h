#ifndef rl_tpl_impl_funcall_h
#define rl_tpl_impl_funcall_h

#include "small.h"

#include "util/number.h"

#define TYPE_GUARD(_type, _args, _n)            \
  if (!is_##_type(_args[_n]))                   \
    return func_arg_type_err                    \

#define INDEX_GUARD(_len, _args, _n)                  \
  TYPE_GUARD(small, _args, _n);                       \
  small_t i = as_small(_args[_n]);                    \
  if (i < 0)                                          \
    i += (_len);                                      \
  if (i < 0 || (size_t)i >= (_len))                   \
    return func_arg_value_err;                        \

#endif
