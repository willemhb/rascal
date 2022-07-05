#include "numutils.h"

uint64_t nextipow2(uint64_t i)
{
    if (i==0) return 1;
    if ((i&(i-1))==0) return i;
    
    if (i&BIT64) return BIT64;

    // repeatedly clear bottom bit
    while (i&(i-1))
        i = i&(i-1);

    return i<<1;
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
