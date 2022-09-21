#ifndef rascal_num_h
#define rascal_num_h

#include "val.h"

// forward declarations -------------------------------------------------------
uint64_t ceil_log2(uint64_t i);

// utility macros -------------------------------------------------------------
#define max(a, b)				\
  ({						\
    typeof(a) _a_ = a;				\
    typeof(b) _b_ = b;				\
    _a_ < _b_ ? _b_ : _a_;			\
  })

#endif
