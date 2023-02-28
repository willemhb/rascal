#include "number.h"

// globals --------------------------------------------------------------------
#define TOP_BIT 0x8000000000000000ul

// API ------------------------------------------------------------------------
uword  dtow(double dbl) {
  return ((ieee64_t)dbl).word;
}

double wtod(uword word) {
  return ((ieee64_t)word).dbl;
}

uword ceil2(uword word) {
  if (word==0) return 1;
    if ((word&(word-1))==0) return word;
    if (word&TOP_BIT) return TOP_BIT;

    // repeatedly clear bottom bit
    while (word&(word-1))
        word = word&(word-1);

    return word<<1;
}
