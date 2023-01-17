#ifndef rascal_util_number_h
#define rascal_util_number_h

#include <tgmath.h>

#include "../common.h"

/* API */
ulong clog2(ulong x);

/* convenience */
#define numcmp( x, y )				\
  ({						\
    typeof(x) _x_ = x;				\
    typeof(y) _y_ = y;				\
    0 - (_x_ < _y_) + (_x_ > _y_);		\
  })

#define min( x, y )				\
  ({						\
    typeof(x) _x_ = x;				\
    typeof(y) _y_ = y;				\
    _x_ > _y_ ? _y_ : _x_;			\
  })

#define max( x, y )				\
  ({						\
    typeof(x) _x_ = x;				\
    typeof(y) _y_ = y;				\
    _x_ < _y_ ? _y_ : _x_;			\
  })

#define isint( f )                              \
  ({                                            \
    typeof(f) _f_ = f;                          \
    ((typeof(f))nearbyint(_f_)) == _f_;         \
  })

#endif
