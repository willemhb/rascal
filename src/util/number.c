#include "util/number.h"

// general numeric utilities
uint64_t ceil2(uint64_t word) {
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

bool ispow2(uint64_t word) {
  return popc(word) == 1;
}

bool ispow64(uint64_t word) {
  return popc(word) == 1 && ctz(word) % 6 == 0;
}

uint64_t log64(uint64_t word) {
  assert(ispow64(word));

  return ctz(word) / 6;
}
