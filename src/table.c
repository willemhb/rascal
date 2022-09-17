#include <string.h>

#include "memory.h"
#include "table.h"
#include "value.h"

// hashing functions ----------------------------------------------------------
#define FNV64_PRIME  0x00000100000001B3ul
#define FNV64_OFFSET 0xcbf29ce484222325ul

Hash hashCstring( const Cstring cstr )
{
  return hashMemory( (const uint8_t*)cstr, strlen(cstr) );
}

Hash hashMemory( const UInt8 *bytes, Arity nBytes )
{
  Hash out = FNV64_OFFSET;
  for (Arity i=0; i<nBytes; i++)
    {
      out ^= bytes[i];
      out *= FNV64_PRIME;
    }

  return out;
}

Hash hashInt( uint64_t key )
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

Hash hashReal( Real key )
{
  return hashInt( AS_VALUE(key, 0) );
}

Hash hashPointer( const Pointer p )
{
  return hashInt( AS_VALUE(p, 0) );
}

Hash mixHash( Hash xHash, Hash yHash )
{
  return hashInt( xHash ^ yHash );
}

// table implementations ------------------------------------------------------
#define TABLE_MIN_NKEYS       8
#define TABLE_RESIZE_PRESSURE 0.75

void initTable( Table *table )
{
  initObject( (Obj*)table, OBJ_TABLE );
  table->length   = 0;
  table->capacity = TABLE_MIN_NKEYS;
  table->data     = ALLOCATE( Tuple*, table->capacity );
}

// initialization -------------------------------------------------------------
