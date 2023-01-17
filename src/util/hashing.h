#ifndef rascal_util_hashing_h
#define rascal_util_hashing_h

#include "../common.h"

/* API */
uhash hashInt( int x );
uhash hashLong(  long x );
uhash hashUlong( ulong x );
uhash hashDouble( double x );
uhash hashPointer( const void *x );
uhash hashString( const char *x );
uhash hashMem( const ubyte *x, usize n );
uhash mixHashes( ulong x, ulong y );

#endif
