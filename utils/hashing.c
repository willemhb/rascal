#include <string.h>
#include <assert.h>
#include "hashing.h"


#define FNV1A_OFFSET32 0x811c9dc5u
#define FNV1A_PRIME32  0x01000193u

#define FNV1A_OFFSET64 0xcbf29ce484222325ul
#define FNV1A_PRIME64  0x00000100000001B3ul 


uint64_t memhash( const unsigned char *buf, size_t n ) {
  uint64_t hash = FNV1A_OFFSET64;

  for (size_t i=0; i<n; i++) {
    hash *= FNV1A_PRIME64;
    hash ^= buf[i];
  }

  return hash;
}

uint64_t strhash( const char *buf ) {
  size_t len = strlen( buf );
  assert( len > 0 );

  return memhash( (const unsigned char*)buf, len );
}

uint32_t int32hash(uint32_t a) {
    a = (a+0x7ed55d16) + (a<<12);
    a = (a^0xc761c23c) ^ (a>>19);
    a = (a+0x165667b1) + (a<<5);
    a = (a+0xd3a2646c) ^ (a<<9);
    a = (a+0xfd7046c5) + (a<<3);
    a = (a^0xb55a4f09) ^ (a>>16);
    return a;
}

uint32_t floathash(float key) {
  uint32_t buf;
  memcpy( &buf, &key, 4);
  return int32hash( buf );
}

uint64_t int64hash(uint64_t key) {
    key = (~key) + (key << 21);            // key = (key << 21) - key - 1;
    key =   key  ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8); // key * 265
    key =  key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4); // key * 21
    key =  key ^ (key >> 28);
    key =  key + (key << 31);
    return key;
}

uint64_t doublehash(double key) {
  uint64_t buf;
  memcpy(&buf, &key, 8);
  return int64hash(buf);
}

uint32_t int64to32hash(uint64_t key)
{
    key = (~key) + (key << 18); // key = (key << 18) - key - 1;
    key =   key  ^ (key >> 31);
    key = key * 21;             // key = (key + (key << 2)) + (key << 4);
    key = key ^ (key >> 11);
    key = key + (key << 6);
    key = key ^ (key >> 22);
    return (uint32_t)key;
}

// cribbed shamelessly from femtolisp
uint64_t pointerhash( void *key ) {
  uintptr_t hash = (uintptr_t)key;

  hash = (~hash) + (hash << 21);            // hash = (hash << 21) - hash - 1;
  hash =   hash  ^ (hash >> 24);
  hash = (hash + (hash << 3)) + (hash << 8); // hash * 265
  hash =  hash ^ (hash >> 14);
  hash = (hash + (hash << 2)) + (hash << 4); // hash * 21
  hash =  hash ^ (hash >> 28);
  hash =  hash + (hash << 31);

  return hash;
}
