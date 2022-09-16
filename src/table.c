#include <string.h>

#include "memory.h"
#include "table.h"
#include "value.h"

// hashing functions ----------------------------------------------------------
#define FNV64_PRIME  0x00000100000001B3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

hash_t hashCstring( const Cstring cstr )
{
  return hashMemory( (const uint8_t*)cstr, strlen(cstr) );
}

hash_t hashMemory( const uint8_t *bytes, int nBytes )
{
  hash_t out = FNV64_OFFSET;
  for (int i=0; i<nBytes; i++)
    {
      out ^= bytes[i];
      out *= FNV64_PRIME;
    }

  return out;
}

hash_t hashInt( uint64_t key )
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

hash_t hashReal( Real key )
{
  return hashInt( AS_VALUE(key, 0) );
}

hash_t hashPointer( const Pointer p )
{
  return hashInt( AS_VALUE(p, 0) );
}

hash_t mixHash( hash_t xHash, hash_t yHash )
{
  return hashInt( xHash ^ yHash );
}

// table implementations ------------------------------------------------------
TABLE_IMPL(Map, Value, Value, MapEntry, hashValue, equalValues)
TABLE_IMPL(Set, Value, Value, SetEntry, hashValue, equalValues)

