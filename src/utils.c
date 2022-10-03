#include <string.h>

#include "utils.h"

// array sizing utilities
static const double table_resize_pressure = 0.75;

size_t pad_alist_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  if (oldc >= newl && newl >= (oldc>>1))			
    return oldc;							
  arity32_t newc = ((size_t)newl+(newl>>3)+6)&~(size_t)3;	
  if (newl - oldl > newc - oldc)				
    newc = ((size_t)newl+3)&~(size_t)3;
  if (newc < minc)
    newc = minc;
  return newc;
}

size_t pad_stack_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  size_t newc = oldc;
  if (newl > oldl)						
    while (newl > newc)				       
      newc <<= 1;
  else if (oldl > newl)
    while (newc > minc && newl < (newc >> 1))
      newc >>= 1;
  if (newc < minc)
    newc = minc;
  return newc;
}

size_t pad_table_size(size_t oldl, size_t newl, size_t oldc, size_t minc)
{
  size_t newc = oldc;

  if (newl > oldl)						
    while (newl > newc*table_resize_pressure)				       
      newc <<= 1;
  else if (oldl > newl)
    while (newc > minc && newl < table_resize_pressure*(newc >> 1))
      newc >>= 1;
  if (newc < minc)
    newc = minc;
  return newc;

}

// numeric & low-level helpers
#define TOP_BIT 0x8000000000000000ul

uint64_t clog2(uint64_t i)
{
  if (i == 0)
    return 1;

  if ((i&(i-1))==0)
    return i;

  if (i&TOP_BIT)
    return TOP_BIT;

  while (i&(i-1))
    i &= i-1;

  return i<<1;
}

// hashing
hash32_t hash_int( uint32_t key )
{
  key = (key+0x7ed55d16) + (key<<12);
  key = (key^0xc761c23c) ^ (key>>19);
  key = (key+0x165667b1) + (key<<5);
  key = (key+0xd3a2646c) ^ (key<<9);
  key = (key+0xfd7046c5) + (key<<3);
  key = (key^0xb55a4f09) ^ (key>>16);

  return key;
}

hash32_t hash_float( float key )
{
  return hash_int( float_bits(key) );
}

hash64_t hash_long( uint64_t key )
{
  key = (~key) + (key << 21);            // key = (key << 21) - key - 1;
    key =   key  ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8); // key * 265
    key =  key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4); // key * 21
    key =  key ^ (key >> 28);
    key =  key + (key << 31);
    return key;
}

hash64_t hash_double( double d )
{
  return hash_int( real_bits(d) );
}

hash64_t hash_ptr( void *p )
{
  return hash_int((uintptr_t)p);
}

hash64_t mix_hashes( hash64_t h1, hash64_t h2 )
{
  return hash_int( h1 ^ h2 );
}

// comparison functions -------------------------------------------------------
int32_t u16cmp(uint16_t *xb, uint16_t *yb, size_t n)
{
  for (uint16_t o;n--; xb++, yb++)
      if ((o=*xb-*yb))
	return (int32_t)o;
  return 0;
}

int32_t u32cmp(uint32_t *xb, uint32_t *yb, size_t n)
{
 for (uint32_t o;n--; xb++, yb++)
      if ((o=*xb-*yb))
	return (int32_t)o;
  return 0;
}

int64_t u64cmp(uint64_t *xb, uint64_t *yb, size_t n)
{
 for (uint64_t o;n--; xb++, yb++)
      if ((o=*xb-*yb))
	return (int64_t)o;
  return 0;
}

// string hashing utilities ---------------------------------------------------
#define FNV64_PRIME  0x00000100000001B3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

hash64_t hash_string( char *chars )
{
  arity32_t n = strlen(chars);
  return hash_bytes((byte_t*)chars, n);
}

hash64_t hash_wbytes( uint32_t *mem, arity32_t n )
{
  hash64_t  out = FNV64_OFFSET;
  
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

bool ihash_wbytes( uint32_t **mem, hash64_t **buf, arity32_t *cnt, arity32_t *cap )
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

hash64_t hash_bytes( byte_t *mem, arity32_t n)
{
  hash64_t  out = FNV64_OFFSET;

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

bool ihash_bytes( byte_t **mem, hash64_t **buf, arity32_t *cnt, arity32_t *cap )
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
