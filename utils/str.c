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

  // hashing word-sized chunks, but ensuring memory is aligned first
      if (n&1)
	{
	  out ^= *mem;
	  out *= FNV64_PRIME;
	  mem += 1;
	  n   &= ~1ul;
	}

      if (n&3)
	{
	  out ^= *(uint16_t*)mem;
	  out *= FNV64_PRIME;
	  mem += 2;
	  n   &= ~2ul;
	}

      if (n&7)
	{
	  out ^= *(uint16_t*)mem;
	  out *= FNV64_PRIME;
	  mem += 4;
	  n   &= ~4ul;
	}

      while (n > 8)
	{
	  out ^= *(uint64_t*)mem;
	  out *= FNV64_PRIME;
	  mem += 8;
	  n   -= 8;
	}


  return out;
}
