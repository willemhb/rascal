#include "hashing.h"


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
