#include "util/number.h"

#define TOP_BIT 0x8000000000000000UL

word_t ceil2(word_t w) {
    if ( w == 0 )
      return 1;

    if ( (w & (w - 1)) == 0 ) // power of 2
      return w;

    if ( w & TOP_BIT )
      return TOP_BIT;

    // repeatedly clear bottom bit
    while ( w & (w - 1) )
        w = w & (w - 1);

    return w << 1;
}
