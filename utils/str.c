#include <string.h>

#include "utils/str.h"

// comparison functions -------------------------------------------------------
int32_t u16cmp(uint16_t *xb, uint16_t *yb, size_t n)
{
  for (uint16_t o;n--; xb++, yb++)
      if ((o=*xb-*yb))
	return (int)o;
  return 0;
}

int32_t u32cmp(uint32_t *xb, uint32_t *yb, size_t n)
{
 for (uint32_t o;n--; xb++, yb++)
      if ((o=*xb-*yb))
	return (int)o;
  return 0;
}

int64_t u64cmp(uint64_t *xb, uint64_t *yb, size_t n)
{
 for (uint64_t o;n--; xb++, yb++)
      if ((o=*xb-*yb))
	return (int)o;
  return 0;
}


// string hashing utilities ---------------------------------------------------
#define FNV64_PRIME  0x00000100000001B3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

hash_t hash_string( char *chars )
{
  arity_t n = strlen(chars);
  return hash_bytes((byte_t*)chars, n);
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

bool ihash_wbytes( uint32_t **mem, hash_t **buf, arity_t *cnt, arity_t *cap )
{
  if (*cnt == 0)
      **buf = FNV64_OFFSET;

  if (*cnt == *cap)
    return false;

  **buf ^= **mem;
  **buf *= FNV64_PRIME;
  (*buf)++;
  (*mem)++;

  return true;
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

bool ihash_bytes( byte_t **mem, hash_t **buf, arity_t *cnt, arity_t *cap )
{
  if (*cnt == 0)
      **buf = FNV64_OFFSET;

  if (*cnt == *cap)
    return false;

  **buf ^= **mem;
  **buf *= FNV64_PRIME;
  (*buf)++;
  (*mem)++;

  return true;
}
