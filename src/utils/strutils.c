#include "utils/strutils.h"


// comparison functions -------------------------------------------------------
int32_t u16cmp(uint16_t *xb, uint16_t *yb, size_t n)
{
  for (uint16_t o;n--; xb++, yb++)
      if ((o=*xb-*yb))
	return (int32_t)o;
  return 0;
}

int32_t u32cmp(uint32_t *xb, uint32_t *yb, size_t n)
{
 for (uint32_t o;n--; xb++, yb++)
      if ((o=*xb-*yb))
	return (int32_t)o;
  return 0;
}

int64_t u64cmp(uint64_t *xb, uint64_t *yb, size_t n)
{
 for (uint64_t o;n--; xb++, yb++)
      if ((o=*xb-*yb))
	return (int64_t)o;
  return 0;
}
