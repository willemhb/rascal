#include "util/number.h"

// general numeric utilities
uint64_t ceil_2(uint64_t word) {
  // stolen from femtolisp

  if (word == 0)
    return 1;

  if ((word & (word - 1)) == 0)
    return word;

  if (word & SIGN)
    return SIGN;

  while (word & (word-1))
    word = word & (word-1);

  return word<<1;
}
