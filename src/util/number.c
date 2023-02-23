#include "util/number.h"

int comp_ints(int x, int y) {
  return x - y;
}

int cmp_reals(double x, double y) {
  return x < y ? -1 : x > y;
}

int cmp_words(uint64 x, uint64 y) {
  return x < y ? -1 : x > y;
}

// misc utilities -------------------------------------------------------------
uint64 ceil2(uint64 n) {
  #define TOP_BIT 0x8000000000000000ul
  
    if (n==0)
      return 1;

    if ((n&(n-1))==0)
      return n;

    if (n&TOP_BIT)
      return TOP_BIT;

    // repeatedly clear bottom bit
    while (n&(n-1))
        n = n&(n-1);

    return n<<1;
}
