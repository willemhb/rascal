#include "util/ieee754.h"


/* external APIs */
double word_to_double(word_t word) {
  return ((ieee_64_t)word).dbl;
}

word_t double_to_word(double dbl) {
  return ((ieee_64_t)dbl).word;
}
