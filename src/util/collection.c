#include "util/number.h"
#include "util/collection.h"

#include "memory.h"

static const size_t minStackCap = 8;
static const size_t minOrdsCap  = 8;
static const double loadFactor  = 0.625; // 5/8
static const double loadFactorC = 1.6;   // 8/5

/* general collection utilities */
bool checkAlistResize(size_t newCnt, size_t oldCap, bool encoded) {
  size_t padded = newCnt + encoded;
  return padded > oldCap || padded < (oldCap >> 1);
}

bool checkTableResize(size_t newCnt, size_t oldCap) {
  size_t padded = loadFactorC * newCnt;

  return padded > oldCap || padded < (oldCap >> 1);
}

size_t padAlistSize(size_t oldCnt, size_t newCnt, size_t oldCap, bool encoded) {
  /* CPython resize algorithm */
  size_t out;

  if (newCnt == 0)
    out = 0;

  else if (checkAlistResize(newCnt, oldCap, encoded)) {
    size_t newCap = (newCnt + (newCnt >> 3) + 6) & ~(size_t)3;

    if (newCnt - oldCnt > newCap - newCnt)
      newCap = (newCnt + 3) & ~(size_t)3;

    out = newCap;
  } else {
    out = oldCap;
  }

  return out;
}

size_t padStackSize(size_t oldCnt, size_t newCnt, size_t oldCap, bool encoded) {
  (void)oldCnt;

  size_t out;

  if (newCnt == 0)
    out = 0;

  else if (checkAlistResize(newCnt, oldCap, encoded))
    out = max(minStackCap, ceilPow2(newCnt));

  else
    out = max(minStackCap, ceilPow2(newCnt));

  return out;
}

size_t padOrdSize(size_t oldCnt, size_t newCnt, size_t oldCap) {
  (void)oldCnt;

  size_t out;

  if (newCnt == 0)
    out = 0;

  else if (checkTableResize(newCnt, oldCap))
    out = max(minOrdsCap, ceilPow2(newCnt));

  else
    out = oldCap;

  return out;
}

size_t getOrdSize(size_t cnt) {
  if (cnt <= INT8_MAX)
    return sizeof(int8_t);

  else if (cnt <= INT16_MAX)
    return sizeof(int16_t);

  else if (cnt <= INT32_MAX)
    return sizeof(int32_t);

  else
    return sizeof(int64_t);
}

void* allocOrds(size_t cap, size_t oSize) {
  if (oSize == 0)
    oSize = getOrdSize(cap);

  size_t mSize = cap*oSize;
  void* out    = allocate(NULL, mSize);

  memset(out, -1, mSize);
  return out;
}
