#ifndef rl_util_hashing_h
#define rl_util_hashing_h

#include "common.h"

/* C types */
typedef uint64_t hash_t;

/* API */
hash_t hash_int32( int32_t x );
hash_t hash_int( int64_t x );
hash_t hash_uint( uint64_t x );
hash_t hash_double( double x );
hash_t hash_pointer( const void *x );
hash_t hash_str8( const char *x );
hash_t hash_mem8( const byte *x, size_t n );
hash_t mix_hashes( uint64_t x, uint64_t y );

#endif
