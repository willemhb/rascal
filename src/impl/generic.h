#ifndef rl_tpl_impl_generic_h
#define rl_tpl_impl_generic_h

#include <assert.h>
#include "type.h"

#undef ISA_METHOD
#undef ASA_METHOD
#undef HEAD_METHOD

#define NOOP_GUARD(x) true
#define NON0_GUARD(x) x

#define NOOP_CNVT(x)  x

#define OBJ_GETFL(x) (obj_flags((obj_t)(x)))

#define ISA_METHOD(type, argtype, getter, n, ...)    \
  bool argtype##_is_##type(TYPE(argtype) argtype) {  \
    return has_rl_types(argtype, n, __VA_ARGS__);    \
  }

#define ISA_NON0(type, argtype)                     \
  bool argtype##_is_##type(TYPE(argtype) argtype) { \
    return !!argtype;                               \
  }

#define ASA_METHOD(type, argtype, guard, cnvt)              \
  TYPE(type) argtype##_as_##type(TYPE(argtype) argtype) {   \
    assert(guard(argtype));                                 \
    return (TYPE(type))cnvt(argtype);                       \
  }

#define HEAD_METHOD(type, argtype, guard, cnvt)                         \
  STRUCT_HEAD(type) *argtype##_##type##_head(TYPE(argtype) argtype) {   \
    assert(guard(argtype));                                             \
    return (STRUCT_HEAD(type)*)(obj_start((obj_t)argtype));             \
  }

#define FLAGP_METHOD(type, argtype, getfl, fl)                         \
  bool argtype##_is_##fl(TYPE(argtype) argtype) {                      \
    return is_##type(argtype) && flagp(getfl(argtype), fl##_##type);   \
  }

#endif
