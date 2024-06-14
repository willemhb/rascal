#include <string.h>

#include "util/hash.h"

/* hashing utilities */
/* Globals */
#define HASH_WIDTH   48
#define HASH_MASK    0x0000ffffffffffffUL
#define FNV64_PRIME  0x00000100000001B3
#define FNV64_OFFSET 0xcbf29ce484222325

hash_t hash_word(word_t w) {
    w = (~w) + (w << 21);           // w = (w << 21) - w - 1;
    w =   w  ^ (w >> 24);
    w =  (w + (w << 3)) + (w << 8); // w * 265
    w =   w ^ (w >> 14);
    w =  (w + (w << 2)) + (w << 4); // w * 21
    w =   w ^ (w >> 28);
    w =   w + (w << 31);

    return w & HASH_WIDTH;          // compress to standard 48-bit Rascal hash
}

hash_t hash_chars(const char* data) {
  return hash_bytes((const byte_t*)data, strlen(data));
}

// TODO: hash in larger chunks
hash_t hash_bytes(const byte_t* data, size_t count) {
  hash_t o = FNV64_OFFSET;

  for ( size_t i=0; i < count; i++ ) {
    o ^= data[i];
    o *= FNV64_PRIME;
  }

  return o & HASH_WIDTH; // compress to standard 48-bit Rascal hash
}

hash_t mix_hashes(hash_t hx, hash_t hy) {
  if ( hx == 0 )
    return hy & HASH_WIDTH;

  if ( hy == 0 )
    return hx & HASH_WIDTH;

  return hash_word(hx ^ hy);
}
