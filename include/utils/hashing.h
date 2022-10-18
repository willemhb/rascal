#ifndef rascal_util_hashing_h
#define rascal_util_hashing_h

#include "common.h"

// basic hashing functions
hash_t hash_wbytes( uint *wchrs, size_t cnt );
hash_t hash_bytes( byte *mem, size_t cnt );
hash_t hash_string( char *chars );
hash_t hash_int( uint32_t u );
hash_t hash_long( uint64_t u );
hash_t hash_float( float f );
hash_t hash_double( double r );
hash_t hash_ptr( void *p );
hash_t mix_hashes( hash_t h1, hash_t h2 );

#endif
