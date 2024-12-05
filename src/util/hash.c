#include <string.h>

#include "util/hash.h"

/* hashing utilities */
/* Globals */
#define HASH_WIDTH   48
#define HASH_MASK    0x0000ffffffffffffUL
#define FNV64_PRIME  0x00000100000001B3
#define FNV64_OFFSET 0xcbf29ce484222325

hash64 hash_word(word_t w) {
    w = (~w) + (w << 21);           // w = (w << 21) - w - 1;
    w =   w  ^ (w >> 24);
    w =  (w + (w << 3)) + (w << 8); // w * 265
    w =   w ^ (w >> 14);
    w =  (w + (w << 2)) + (w << 4); // w * 21
    w =   w ^ (w >> 28);
    w =   w + (w << 31);

    w &= HASH_MASK; // compress

    return w;
}

hash64 hash_chars(const char* data, size64 n) {
  if ( n == 0 )
    n = strlen(data);
  
  return hash_bytes((const byte*)data, n);
}

// TODO: hash in larger chunks
hash64 hash_bytes(const byte* data, size_t n) {
  hash64 o = FNV64_OFFSET;

  for ( size_t i=0; i < n; i++ ) {
    o ^= data[i];
    o *= FNV64_PRIME;
  }
  
  o &= HASH_MASK; // compress
  
  return o;
}

hash64 hash_pointer(const void* p) {
  return hash_word((word_t)p);
}

hash64 mix_hashes(hash64 hx, hash64 hy) {
  hash64 o;
  
  if ( hx == 0 )
    o = hy;

  else if ( hy == 0 )
    o = hx;

  else
    o = hash_word(hx ^ hy);

  o &= HASH_MASK; // compress

  return o;
}

#undef HASH_WIDTH
#undef HASH_MASK
#undef FNV64_PRIME
#undef FNV64_OFFSET
