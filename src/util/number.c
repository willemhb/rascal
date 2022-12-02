#include "util/number.h"

#define TOP_BIT 0x8000000000000000ul

uint64_t clog2( uint64_t i )
{
  if (i==0) return 1;
    if ((i&(i-1))==0) return i;
    if (i&TOP_BIT) return TOP_BIT;

    // repeatedly clear bottom bit
    while (i&(i-1))
        i = i&(i-1);

    return i<<1;
}
