#ifndef rascal_util_collection_h
#define rascal_util_collection_h

#include "../common.h"

static const int MinCap = 8;
static const int MaxCap = INT32_MAX;
static const double TablePressure = 0.625;

#define ASSERT_BOUND(size, min, max)			\
  do {							\
    __typeof__(size) __size = size;			\
    assert(__size >= (min) && __size <= (max));		\
  } while(false)

#define ASSERT_INDEX(index, length)		\
  ASSERT_BOUND(index, 0, (length)-1)

/* common utilities for working arrays, alists, tables, etc. */
usize padArraySize(usize newCount, usize oldCap);
usize padStringSize(usize newCount, usize oldCap);
usize padAlistSize(usize newCount, usize oldCap);
usize padBufferSize(usize newCount, usize oldCap);
usize padTableSize(usize newCount, usize oldCap);

#endif
