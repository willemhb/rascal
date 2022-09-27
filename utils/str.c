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

int u32cmp(uint32_t *xb, uint32_t *yb, size_t n)
{
  uint32_t *xe = xb+n,  *ye = yb+n;

  uint32_t o;

  for (uint32_t *xc=xb, *yc=yb; xc<xe && yc<ye; xc++, yc++)
      if ((o=*xc-*yc))
	return (int)o;

  return 0;
}

hash_t hash_wbytes( uint32_t *mem, arity_t n )
{
  hash_t  out = FNV64_OFFSET;
  
  if ((uint64_t)mem & 7)
    {
      out ^= *mem;
      out *= FNV64_PRIME;
      mem++;
      n--;
    }
  while (n > 2)
    {
      out ^= *(uint64_t*)mem;
      out *= FNV64_PRIME;
      mem += 2;
      n   -= 2;
    }
  if (n)
    {
      out ^= *mem;
      out *= FNV64_PRIME;
    }
  return out;
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
