#include "number.h"

// API ------------------------------------------------------------------------
uword  dtow(double dbl) {
  return ((ieee64_t)dbl).word;
}

double wtod(uword word) {
  return ((ieee64_t)word).dbl;
}
