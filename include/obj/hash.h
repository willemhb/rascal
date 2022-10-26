#ifndef rascal_obj_hash_h
#define rascal_obj_hash_h

#include "obj/table.h"

// data structures required for hashing go here
// for implementation of hash, see rl/hash.h

// these data structures are used to resolve cycles
// in data structures during hashing
typedef struct hash_cache_t hash_cache_t;
typedef struct hash_cache_entry_t hash_cache_entry_t;

struct hash_cache_t
{
  TABLE;
};

struct hash_cache_entry_t
{
  OBJECT;
  hash_t    hash;  // this is the hash
  pointer_t key;   // though stored as a pointer, this is necessarily a pointer to a rascal object.
  hash_t    value; // 
};

// globals
extern type_t HashCacheType, HashCacheEntryType;

#endif
