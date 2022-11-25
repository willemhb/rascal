#ifndef rl_util_number_h
#define rl_util_number_h

#include "common.h"

/* commentary */

/* C types */

/* globals */

/* API */
uint64_t clog2( uint64_t x );

/* runtime */

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

#endif
