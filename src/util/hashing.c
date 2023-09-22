#include "util/hashing.h"

// utilities
uint64_t ceilPow2(uint64_t word) {
  // stolen from femtolisp

  if (word == 0)
    return 1;

  if ((word & (word - 1)) == 0)
    return word;

  if (word & SIGN)
    return SIGN;

  while (word & (word-1))
    word = word & (word-1);

  return word;
}

// hashing functions
#define FNV_PRIME_64  0x00000100000001B3UL
#define FNV_OFFSET_64 0xCBF29CE484222325UL

uint64_t hashString(const char* chars) {
  uint64_t hash = FNV_OFFSET_64;

  for (char ch = *chars; ch != '\0'; chars++, ch=*chars ) {
    hash ^= ch;
    hash *= FNV_PRIME_64;
  }

  return hash;
}

uint64_t hashBytes(const uint8_t* bytes, size_t n) {
  uint64_t hash = FNV_OFFSET_64;

  for (size_t i=0; i<n; i++) {
    hash ^= bytes[i];
    hash *= FNV_PRIME_64;
  }

  return hash;
}

uint64_t hashWord(uint64_t word) {
  // stolen from femtolisp.
  word = (~word) + (word << 21);             // word = (word << 21) - word - 1;
  word =   word  ^ (word >> 24);
  word = (word + (word << 3)) + (word << 8); // word * 265
  word =  word ^ (word >> 14);
  word = (word + (word << 2)) + (word << 4); // word * 21
  word =  word ^ (word >> 28);
  word =  word + (word << 31);
  return word;  
}

uint64_t hashPtr(const void* pointer) {
  return hashWord((uint64_t)pointer);
}

uint64_t hashDouble(double num) {
  return hashWord(doubleToWord(num));
}
