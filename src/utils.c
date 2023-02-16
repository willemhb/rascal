#include <string.h>

#include "utils.h"
#include "value.h"


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
  return hash_uint(mk_real(x));
}

uhash hash_ptr(const void *x) {
  return hash_uint((uint64)x);
}

uhash hash_str(const char *xs) {
  return hash_mem((const ubyte*)xs, strlen(xs));
}

uhash hash_mem(const ubyte *xs, usize n) {
  uhash h = FNV64_OFFSET;

  for (usize i=0; i<n; i++) {
    h ^= xs[i];
    h *= FNV64_PRIME;
  }

  return h;
}

uhash mix_hashes(uhash hx, uhash hy) {
  return hash_uint(hx ^ hy);
}
