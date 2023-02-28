#include <string.h>
#include <stdarg.h>

#include "hashing.h"
#include "number.h"

// globals --------------------------------------------------------------------
#define FNV64_PRIME  0x00000100000001B3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

// API ------------------------------------------------------------------------
uword hash_uword(uword word) {
  word = (~word) + (word << 21);             // word = (word << 21) - word - 1;
  word =   word  ^ (word >> 24);
  word = (word + (word << 3)) + (word << 8); // word * 265
  word =  word ^ (word >> 14);
  word = (word + (word << 2)) + (word << 4); // word * 21
  word =  word ^ (word >> 28);
  word =  word + (word << 31);
  return word;
}

uhash hash_double(double dbl) {
  return hash_uword(dtow(dbl));
}

uhash hash_pointer(void* ptr) {
  return hash_uword((uhash)ptr);
}

uhash hash_str(char* chars) {
  return hash_mem((ubyte*)chars, strlen(chars));
}

uhash hash_mem(ubyte* bytes, usize n) {
  uhash h = FNV64_OFFSET;

  for (usize i=0; i<n; i++) {
    h ^= bytes[i];
    h *= FNV64_PRIME;
  }

  return h;
}

uhash mix_2_hashes(uhash x, uhash y) {
  return hash_uword(x ^ y);
}

uhash mix_3_hashes(uhash x, uhash y, uhash z) {
  uhash w = hash_uword(x ^ y);
  return hash_uword(w ^ z);
}

uhash mix_n_hashes(usize n, ...) {
  assert(n > 1);

  va_list va; va_start(va, n);
  
  uhash h = va_arg(va, uhash);

  for (usize i=1; i<n; i++)
    h = mix_2_hashes(h, va_arg(va, uhash));

  va_end(va);

  return h;
}
