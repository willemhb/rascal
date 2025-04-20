#include "util.h"

// miscellaneous bit twiddling
#define TOP_BIT 0x8000000000000000ul

uintptr_t cpow2(uintptr_t i)
{
    if ( i == 0 )
      return 1;
    if ( (i & (i-1)) == 0 )
      return i;
    if ( i &TOP_BIT )
      return TOP_BIT;

    // bit twiddling (changed from busted version using while loop)
    return i << (64 - clz(i) + 1);
}

// hashing constants
#define FNV64_PRIME  0x100000001b3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

// hashing functions
hash_t hash_string(char* chars) {
  hash_t hash = FNV64_OFFSET;

  while ( (*chars) != '\0' ) {
    hash = hash ^ *chars;
    hash = hash * FNV64_PRIME;
    chars++;
  }

  return hash;
}

hash_t hash_word(uintptr_t word) {
  // copied directly from femtolisp repo
    word = (~word) + (word << 21);             // word = (word << 21) - word - 1;
    word =   word  ^ (word >> 24);
    word = (word + (word << 3)) + (word << 8); // word * 265
    word =  word ^ (word >> 14);
    word = (word + (word << 2)) + (word << 4); // word * 21
    word =  word ^ (word >> 28);
    word =  word + (word << 31);
    return word;
}

hash_t hash_pointer(void* ptr) {
  return hash_word((uintptr_t)ptr);
}
