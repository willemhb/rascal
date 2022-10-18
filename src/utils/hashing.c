#include <string.h>

#include "utils/hashing.h"
#include "utils/numutils.h"

 
// hashing
hash_t hash_int( uint32_t key )
{
  key = (key+0x7ed55d16) + (key<<12);
  key = (key^0xc761c23c) ^ (key>>19);
  key = (key+0x165667b1) + (key<<5);
  key = (key+0xd3a2646c) ^ (key<<9);
  key = (key+0xfd7046c5) + (key<<3);
  key = (key^0xb55a4f09) ^ (key>>16);

  return key;
}

hash_t hash_float( float key )
{
  return hash_int( float_bits(key) );
}

hash_t hash_long( uint64_t key )
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

hash_t hash_double( double d )
{
  return hash_int( real_bits(d) );
}

hash_t hash_ptr( void *p )
{
  return hash_int((uintptr_t)p);
}

hash_t mix_hashes( hash64_t h1, hash64_t h2 )
{
  return hash_int( h1 ^ h2 );
}

// string hashing utilities ---------------------------------------------------
#define FNV64_PRIME  0x00000100000001B3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

hash64_t hash_string( char *chars )
{
  size_t n = strlen(chars);
  return hash_bytes((byte*)chars, n);
}

hash64_t hash_wbytes( uint32_t *mem, size_t n )
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
