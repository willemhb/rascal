#ifndef rascal_memory_h
#define rascal_memory_h

#include "common.h"

// forward declarations -------------------------------------------------------
// safe API for stdlib allocators ---------------------------------------------
void *safeMalloc( const char *fname, size_t nBytes );
void *safeCalloc( const char *fname, size_t count, size_t obSize );
void *safeResize( const char *fname, void *ptr, size_t nBytes );
void  safeFree( const char *fname, void *ptr );

#endif
