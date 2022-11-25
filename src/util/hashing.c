
#include "vm/value.h"

#include "util/hashing.h"
#include "util/string.h"

/* commentary */

/* C types */

/* globals */
#define FNV_64_OFFSET 14695981039346656037ul
#define FNV_64_PRIME  1099511628211ul

/* API */
hash_t hash_int( int64_t x )
{
  return hash_uint((uint64_t)x);
}

hash_t hash_double( double x )
{
  return hash_uint(((rl_data_t)x).as_real);
}

hash_t hash_pointer( const void *x )
{
  return hash_uint((uintptr_t)x);
}

hash_t hash_str8( const char *x )
{
  return hash_mem8((const byte*)x, strlen8(x));
}

hash_t hash_mem8( const byte *x, size_t n )
{
  hash_t hash = FNV_64_OFFSET;

  for ( size_t i=0; i<n; i++ )
    {
      hash ^= x[i];
      hash *= FNV_64_PRIME;
    }

  return hash;
}

hash_t hash_uint( uint64_t x )
{
  x = (~x) + (x << 21);            // x = (x << 21) - x - 1;
  x =   x  ^ (x >> 24);
  x = (x + (x << 3)) + (x << 8); // x * 265
  x =  x ^ (x >> 14);
  x = (x + (x << 2)) + (x << 4); // x * 21
  x =  x ^ (x >> 28);
  x =  x + (x << 31);
  return x;
}

hash_t mix_hashes( uint64_t x, uint64_t y )
{
  return hash_uint(x ^ y);
}

/* runtime */

/* convenience */
