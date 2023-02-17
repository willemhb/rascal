#ifndef util_number_h
#define util_number_h

#include "common.h"

/* API */
// numeric comparisons --------------------------------------------------------
int cmp_ints(int x, int y);
int cmp_reals(double x, double y);
int cmp_words(uint64 x, uint64 y);

// generic min/max ------------------------------------------------------------
#define MAX(x, y)				\
  ({						\
    __auto_type _x = x;				\
    __auto_type _y = y;				\
    _x < _y ? _y : _x;				\
  })

#define MIN(x, y)				\
  ({						\
    __auto_type _x = x;				\
    __auto_type _y = y;				\
    _x > _y ? _y : _x;				\
  })

#endif
