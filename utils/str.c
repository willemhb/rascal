#include <string.h>

#include "utils/str.h"

// string functions -----------------------------------------------------------
#define FNV64_PRIME  0x00000100000001B3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

hash_t hash_string( char *chars )
{
  arity_t n = strlen(chars);
  return hash_bytes((byte_t*)chars, n);
}

hash_t hash_bytes( byte_t *mem, arity_t n)
{
  hash_t  out = FNV64_OFFSET;

  while (n > 8)
    {
      uint64_t chunk  = *(uint64_t*)mem;
      out            ^= chunk;
      out            *= FNV64_PRIME;
      mem            += 8;
      n              -= 8;
    }

  for (arity_t i=0; i<n; i++)
    {
      out ^= mem[i];
      out *= FNV64_PRIME;
    }

  return out;
}
