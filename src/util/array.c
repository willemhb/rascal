#include <stdarg.h>
#include <string.h>

#include "obj/array.h"

static const Size minCap = 8;

// describe macros

// general array utilities
Size padArrayLength( Size newCount, Size oldCap )
{
  Size newCap = oldCap;

  while ( newCount < newCap/2 ) newCap >>= 1;
  while ( newCount > newCap ) newCap <<= 1;

  newCap = max(newCap, minCap);

  return newCap;
}
