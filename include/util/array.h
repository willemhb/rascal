#ifndef rl_util_array_h
#define rl_util_array_h

#include "common.h"

/* commentary

   bounds checked array types and operations
   on those types.
 */

/* C types */

#define array( X, ... )				\
  struct					\
  {						\
    size_t len;					\
    size_t cap;					\
    X elements[__VA_ARGS__];			\
  }

/* globals */

/* API */

/* runtime */

/* convenience */

#endif
