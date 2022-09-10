#include "numutils.h"


uint64_t intceil( uint64_t u, uint64_t div )
{
  return u / div + !!(u % div);
}

uint64_t intfloor( uint64_t u, uint64_t div )
{
  return u / div;
}

size_t cntlzl( uint64_t u )
{
  return __builtin_clzl( u );
}

size_t cnttzl( uint64_t u )
{
  return __builtin_ctzl( u );
}

size_t popcnt( uint32_t u )
{
  return __builtin_popcount( u );
}

size_t popcntl( uint64_t u )
{
  return __builtin_popcountl( u );
}

uint64_t ceil_log2(uint64_t i)
{
    if (i==0)
      return 1;
    if ((i&(i-1))==0)
      return i;
    
    if (i&BIT64)
      return BIT64;

    // repeatedly clear bottom bit
    while (i&(i-1))
        i = i&(i-1);

    return i<<1;
}

uint64_t ceil_log32( uint64_t i )
{
  if (i==0)
    return 1;

  if (i&(0x7ul<<61)) // largest supported power of 32
    return 0x1ul<<60;

  size_t topbit = cntlzl( i );
  size_t shift  = 5 * intceil( topbit, 5 );
  return 1<<shift;
}

inline int ord_int(int ix, int iy) {
  return 0 - (ix < iy) + (ix > iy);
}

inline int ord_uint(uint32_t ux, uint32_t uy) {
  return 0 - (ux < uy) + (ux > uy);
}

inline int ord_ulong( uintptr_t ux, uintptr_t uy ) {
  return 0 - (ux < uy) + (ux > uy);
}
