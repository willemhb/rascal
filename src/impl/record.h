#ifndef rl_tpl_impl_record_h
#define rl_tpl_impl_record_h

#include <assert.h>

#include "tpl/decl/generic.h"

#undef GET
#undef SET

#define GET(T, F, X)				\
  X get_##T##_##F( TYPE(T) *T )			\
  {						\
    assert(T);					\
    return T->F;				\
  }

#define SET(T, F, X)				\
  X set_##T##_##F( TYPE(T) *T, X F )		\
  {						\
    assert(T);					\
    T->F = F;					\
    return F;					\
  }

#endif
