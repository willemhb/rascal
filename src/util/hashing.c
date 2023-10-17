#include "util/hashing.h"

// hashing functions
#define FNV_PRIME_64  0x00000100000001B3UL
#define FNV_OFFSET_64 0xCBF29CE484222325UL

hash_t hash_string(const char* chars) {
  uint64_t hash = FNV_OFFSET_64;

  for (char ch=*chars; ch != '\0'; chars++, ch=*chars) {
    hash ^= ch;
    hash *= FNV_PRIME_64;
  }

  return hash;
}

hash_t hash_bytes(const uint8_t* bytes, size_t n) {
  uint64_t hash = FNV_OFFSET_64;

  for (size_t i=0; i<n; i++) {
    hash ^= bytes[i];
    hash *= FNV_PRIME_64;
  }

  return hash;
}

uint64_t hash_word(uint64_t word) {
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

hash_t hash_ptr(const void* pointer) {
  return hash_word((uint64_t)pointer);
}

hash_t hash_double(double num) {
  return hash_word(double_to_word(num));
}

hash_t mix_hashes(hash_t hx, hash_t hy) {
  return hash_word(hx ^ hy);
}
