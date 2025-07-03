/* Hashing utilities. */
// headers --------------------------------------------------------------------
#include "util/hashing.h"

// macros ---------------------------------------------------------------------
#define FNV64_PRIME  0x100000001b3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

// C types --------------------------------------------------------------------

// globals --------------------------------------------------------------------

// function prototypes --------------------------------------------------------

// function implementations ---------------------------------------------------
// internal -------------------------------------------------------------------

// external -------------------------------------------------------------------
hash_t hash_string(const char* chars) {
  hash_t hash = FNV64_OFFSET;

  while ( *chars != '\0' ) {
    hash = hash ^ *chars;
    hash = hash * FNV64_PRIME;
    chars++;
  }

  return hash;
}

hash_t hash_word(uptr_t word) {
  // copied directly from femtolisp repo, no idea how this works
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
  return hash_word((uptr_t)ptr);
}

// initialization -------------------------------------------------------------
