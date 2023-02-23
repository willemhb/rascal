#include <string.h>
#include <stdarg.h>

#include "util/hash.h"

// C types --------------------------------------------------------------------
union fbits {
  double as_double;
  uint64 as_uint64;
  struct {
    uint64 frac : 52;
    uint64 expt : 11;
    uint64 sign :  1;
  } as_parts;
};

// hashing --------------------------------------------------------------------
#define FNV64_PRIME  0x00000100000001B3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

/* API */
uhash hash_uint(uint64 x) {
  // credit authors of FemtoLisp
  x = (~x) + (x << 21);            // x = (x << 21) - x - 1;
  x =  x  ^ (x >> 24);
  x = (x + (x << 3)) + (x << 8); // x * 265
  x =  x ^ (x >> 14);
  x = (x + (x << 2)) + (x << 4); // x * 21
  x =  x ^ (x >> 28);
  x =  x + (x << 31);

  return x;
}

uhash hash_float(double x) {
  return hash_uint(((union fbits)x).as_uint64);
}

uhash hash_pointer(const void *x) {
  return hash_uint((uint64)x);
}

uhash hash_string(const char *xs) {
  return hash_memory((const ubyte*)xs, strlen(xs));
}

uhash hash_memory(const ubyte *xs, usize n) {
  uhash h = FNV64_OFFSET;

  for (usize i=0; i<n; i++) {
    h ^= xs[i];
    h *= FNV64_PRIME;
  }

  return h;
}

uhash mix_hashes(usize n, ...) {
  va_list va; va_start(va, n);

  uhash hx = 0;

  for (usize i=0; i<n; i++)
    hx = hash_uint(hx ^ va_arg(va, uint64));

  va_end(va);
  
  return hx;
}
