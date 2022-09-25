#include "utils/num.h"

size_t sizeof_Ctype( Ctype_t C )
{
  switch(C)
    {
    case Ctype_sint8  ... Ctype_utf8:    return 1;
    case Ctype_sint16 ... Ctype_utf16:   return 2;
    case Ctype_sint32 ... Ctype_float32: return 4;
    default:                             return 8;
    }
}

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

// numeric hashing utilities
hash_t hash_int( uint64_t key )
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

hash_t hash_real( real_t r )
{
  return hash_int( real_bits(r) );
}

hash_t hash_ptr( ptr_t p )
{
  return hash_int((uintptr_t)p);
}

hash_t mix_hashes( hash_t h1, hash_t h2 )
{
  return hash_int( h1 ^ h2 );
}
