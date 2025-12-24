#include <string.h>

#include "util/util.h"


// miscellaneous string helpers
bool streq(char* sx, char* sy) {
  return strcmp(sx, sy) == 0;
}

bool endswith(char* str, char* substr) {
  char* found = strstr(str, substr);

  if ( found == NULL )
    return false;

  return strlen(found) == strlen(substr);
}

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


// miscellaneous numeric helpers
bool is_int(Num n) {
  return n - ((long)n) == 0;
}

// bitmap utilities
#define BITMAP_MASK 0xfffffffffffffffful

bool bitmap_has(uintptr_t map, int n) {
  if ( n < 0 || n > MAX_FARGC )
    return false;

  return (1ul << n) & map;
}

void bitmap_set(uintptr_t* map, int n) {
  assert(n >= 0 && n < MAX_FARGC);
  *map |= 1ul << n;
}

void bitmap_unset(uintptr_t* map, int n) {
  assert(n >= 0 && n < MAX_FARGC);
  *map &= BITMAP_MASK & ~(1ul << n);
}

int bitmap_to_index(uintptr_t map, int n) {
  if ( !bitmap_has(map, n) )
    return -1;

  return popc(((1ul << n) -1) & map);
}

// hashing constants
#define FNV64_PRIME  0x100000001b3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

// hashing functions
hash_t hash_string(const char* chars) {
  hash_t hash = FNV64_OFFSET;

  while ( *chars != '\0' ) {
    hash = hash ^ *chars;
    hash = hash * FNV64_PRIME;
    chars++;
  }

  return hash;
}

hash_t hash_word(uintptr_t word) {
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
  return hash_word((uintptr_t)ptr);
}
