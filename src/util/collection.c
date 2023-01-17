#include <assert.h>

#include "collection.h"

#define ASSERT_SIZE(newSize, oldCap)		\
  do {                                      \
    ASSERT_BOUND(newSize, 0, MaxCap);		\
    ASSERT_BOUND(oldCap, 0, MaxCap);		\
  } while (false)

int padArray(int newSize, int oldCap) {
  /* noOp for passing to ALIST macro */
  ASSERT_SIZE(newSize, oldCap);
  return newSize;
}

int padString(int newSize, int oldCap) {
  /* include space for nul terminator*/
  ASSERT_SIZE(newSize, oldCap);
  return newSize+1;
}

int padAlist(int newSize, int oldCap) {
  ASSERT_SIZE(newSize, oldCap);
  int newCap = oldCap < MinCap ? MinCap : oldCap;

  if (newSize > newCap) {
    while (newCap < newSize) {
      if (newCap == MaxCap)
	break;
      newCap <<= 1; // newCap * 2
    }
  } else if (newSize < newCap / 2) {
    while (newSize < newCap / 2) {
      if (newCap == MinCap)
	break;

      newCap >>= 1; // newCap / 2
    }
  }

  return newCap;
}

int padBuffer(int newSize, int oldCap) {
  /* ensures that a nul character is never unintentionally overwritten */
  ASSERT_SIZE(newSize, oldCap);

  return padAlist(newSize+1, oldCap);
}

int padTable(int newSize, int oldCap) {
  ASSERT_SIZE(newSize, oldCap);

  int newCap = oldCap < MinCap ? MinCap : oldCap;
  
  if (newSize > newCap * TablePressure) {
    while (newCap < newSize * TablePressure) {
      if (newCap == MaxCap)
	break;
      newCap <<= 1;
    }
  } else if (newSize < newCap * TablePressure / 2) {
    while (newSize < newCap * TablePressure / 2) {
      if (newCap == MinCap)
        break;
      
      newCap >>= 1;
    }
  }
  
  return newCap;
}
