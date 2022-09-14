#include "utils.h"

#define TOP_BIT ((uint64_t)0x8000000000000000)

uint64_t ceil_log2( uint64_t i )
{
  if (i==0)
    return 1;
  if ((i&(i-1))==0) // is a power of 2
    return i;
  if (i&TOP_BIT)
    return TOP_BIT;

  while (i&(i-1))
    i = i&(i-1);

  return i<<1;
}
